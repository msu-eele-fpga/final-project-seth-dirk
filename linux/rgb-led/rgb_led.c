#include <linux/module.h>                   // Basic kernel module definitions
#include <linux/platform_device.h>          // Platfrom driver/device definitions
#include <linux/mod_devicetable.h>          // of_device_id and MODULE_DEVICE_TABLE
#include <linux/io.h>                       // iowrite32/ioread32 functions
#include <linux/mutex.h>                    // mutex definitions
#include <linux/miscdevice.h>               // miscdevice definitions
#include <linux/types.h>                    // data types
#include <linux/fs.h>                       // copy_to_user
#include <linux/kstrtox.h>                  // kstrtou32

#define RED_DUTY_OFFSET         0x00            // 0 byte offset for the red duty cycle register
#define GREEN_DUTY_OFFSET       0x04            // 4 byte offset for the green duty cycle register
#define BLUE_DUTY_OFFSET        0x08            // 8 byte offset for the blue duty cycle register
#define PERIOD_OFFSET           0x0C            // 12 byte offset for the period register
#define SPAN 16                                 // Span of the components memory space
/**
* struct rgb_led_dev - Private RGB controller device struct.
* @red_duty_cycle: Address of the red duty cycle register
* @green_duty_cycle: Address of the green duty cycle register
* @blue_duty_cycle: Address of the blue duty cycle register
* @period: Address of the period register
* @miscdev: miscdevice used to create a character device
* @lock: mutex used to prevent concurrent writes to memory
*
* An rgb_led struct gets created for each RGB controller component.
*/
struct rgb_led_dev {
    void __iomem *base_addr;
    void __iomem *red_duty_cycle;
    void __iomem *green_duty_cycle;
    void __iomem *blue_duty_cycle;
    void __iomem *period;
    struct miscdevice miscdev;
    struct mutex lock;
};

/**
* rgb_led_read() - Read method for the rgb_led char device
* @file: Pointer to the char device file struct.
* @buf: User-space buffer to read the value into.
* @count: The number of bytes being requested.
* @offset: The byte offset in the file being read from.
*
* Return: On success, the number of bytes written is returned and the
* offset @offset is advanced by this number. On error, a negative error
* value is returned.
*/
static ssize_t rgb_led_read(struct file *file, char __user *buf,
    size_t count, loff_t *offset)
{
    size_t ret;
    u32 val;

    /*
    * Get the device's private data from the file struct's private_data
    * field. The private_data field is equal to the miscdev field in the
    * rgb_led_dev struct. container_of returns the
    * rgb_led_dev struct that contains the miscdev in private_data.
    */
    struct rgb_led_dev *priv = container_of(file->private_data,
                                struct rgb_led_dev, miscdev);

    // Check file offset to make sure we are reading from a valid location.
    if (*offset < 0) {
        // We can't read from a negative file position.
        return -EINVAL;
    }
    if (*offset >= SPAN) {
        // We can't read from a position past the end of our device.
        return 0;
    }
    if ((*offset % 0x4) != 0) {
        // Prevent unaligned access.
        pr_warn("rgb_led_read: unaligned access\n");
        return -EFAULT;
    }

    val = ioread32(priv->base_addr + *offset);

    // Copy the value to userspace.
    ret = copy_to_user(buf, &val, sizeof(val));
    if (ret == sizeof(val)) {
        pr_warn("rgb_led_read: nothing copied\n");
        return -EFAULT;
    }

    // Increment the file offset by the number of bytes we read.
    *offset = *offset + sizeof(val);

    return sizeof(val);
}

/**
* rgb_led_write() - Write method for the rgb_led char device
* @file: Pointer to the char device file struct.
* @buf: User-space buffer to read the value from.
* @count: The number of bytes being written.
* @offset: The byte offset in the file being written to.
*
* Return: On success, the number of bytes written is returned and the
* offset @offset is advanced by this number. On error, a negative error
* value is returned.
*/
static ssize_t rgb_led_write(struct file *file, const char __user *buf,
    size_t count, loff_t *offset)
{
    size_t ret;
    u32 val;

    struct rgb_led_dev *priv = container_of(file->private_data,
                                struct rgb_led_dev, miscdev);

    if (*offset < 0) {
        return -EINVAL;
    }
    if (*offset >= SPAN) {
        return 0;
    }
    if ((*offset % 0x4) != 0) {
        pr_warn("rgb_led_write: unaligned access\n");
        return -EFAULT;
    }

    mutex_lock(&priv->lock);

    // Get the value from userspace.
    ret = copy_from_user(&val, buf, sizeof(val));
    if (ret != sizeof(val)) {
        iowrite32(val, priv->base_addr + *offset);

        // Increment the file offset by the number of bytes we wrote.
        *offset = *offset + sizeof(val);

        // Return the number of bytes we wrote.
        ret = sizeof(val);
    }
    else {
        pr_warn("rgb_led_write: nothing copied from user space\n");
        ret = -EFAULT;
    }

    mutex_unlock(&priv->lock);
    return ret;
}

/**
* rgb_led_fops - File operations supported by the
* rgb_led driver
* @owner: The rgb_led driver owns the file operations; this
* ensures that the driver can't be removed while the
* character device is still in use.
* @read: The read function.
* @write: The write function.
* @llseek: We use the kernel's default_llseek() function; this allows
* users to change what position they are writing/reading to/from.
*/
static const struct file_operations rgb_led_fops = {
    .owner = THIS_MODULE,
    .read = rgb_led_read,
    .write = rgb_led_write,
    .llseek = default_llseek,
};

/**
* rgb_led_probe() - Initialize device when a match is found
* @pdev: Platform device structure associated with our rgb control device;
* pdev is automatically created by the driver core based upon our
* rgb control device tree node.
*
* When a device that is compatible with this rgb control driver is found, the
* driver's probe function is called. This probe function gets called by the
* kernel when an rgb_led device is found in the device tree.
*/
static int rgb_led_probe(struct platform_device *pdev)
{
    struct rgb_led_dev *priv;
    size_t ret;

    /*
    * Allocate kernel memory for the rgb control device and set it to 0.
    * GFP_KERNEL specifies that we are allocating normal kernel RAM;
    * see the kmalloc documentation for more info. The allocated memory
    * is automatically freed when the device is removed.
    */
    priv = devm_kzalloc(&pdev->dev, sizeof(struct rgb_led_dev),
                        GFP_KERNEL);
    if (!priv) {
        pr_err("Failed to allocate memory\n");
        return -ENOMEM;
    }

    /*
    * Request and remap the device's memory region. Requesting the region
    * make sure nobody else can use that memory. The memory is remapped
    * into the kernel's virtual address space because we don't have access
    * to physical memory locations.
    */
    priv->base_addr = devm_platform_ioremap_resource(pdev, 0);
    if (IS_ERR(priv->base_addr)) {
        pr_err("Failed to request/remap platform device resource\n");
        return PTR_ERR(priv->base_addr);
    }

    // Set the memory addresses for each register.
    priv->red_duty_cycle = priv->base_addr + RED_DUTY_OFFSET;
    priv->green_duty_cycle = priv->base_addr + GREEN_DUTY_OFFSET;
    priv->red_duty_cycle = priv->base_addr + BLUE_DUTY_OFFSET;
    priv->period = priv->base_addr + PERIOD_OFFSET;

    // Set the period to 1 ms and each duty cycle to 0 to begin
    iowrite32(1, priv->period);
    iowrite32(0x0, priv->red_duty_cycle);
    iowrite32(0x0, priv->green_duty_cycle);
    iowrite32(0x0, priv->blue_duty_cycle);

    // Initialize the misc device parameters
    priv->miscdev.minor = MISC_DYNAMIC_MINOR;
    priv->miscdev.name = "rgb_led";
    priv->miscdev.fops = &rgb_led_fops;
    priv->miscdev.parent = &pdev->dev;

    // Register the misc device; this creates a char dev at /dev/rgb_led
    ret = misc_register(&priv->miscdev);
    if (ret) {
        pr_err("Failed to register misc device");
        return ret;
    }

    /* Attach the rgb controller's private data to the platform device's struct.
    * This is so we can access our state container in the other functions.
    */
    platform_set_drvdata(pdev, priv);

    pr_info("rgb_led_probe successful\n");

    return 0;
}

/**
*rgb_led_remove() - Remove an rgb control device.
* @pdev: Platform device structure associated with our rgb control device.
*
* This function is called when an rgb control device is removed or
* the driver is removed.
*/
static int rgb_led_remove(struct platform_device *pdev)
{
    // Get the rgb control's private data from the platform device.
    struct rgb_led_dev *priv = platform_get_drvdata(pdev);

    // Deregister the misc device and remove the /dev/rgb_led file.
    misc_deregister(&priv->miscdev);

    pr_info("rgb_led_remove successful\n");

    return 0;
}

/*
* Define the compatible property used for matching devices to this driver,
* then add our device id structure to the kernel's device table. For a device
* to be matched with this driver, its device tree node must use the same
* compatible string as defined here.
*/
static const struct of_device_id rgb_led_of_match[] = {
    { .compatible = "Howard,rgb_led", },
    { }
};
MODULE_DEVICE_TABLE(of, rgb_led_of_match);

/**
* red_duty_cycle_show() - Return the red duty cycle value to user-space via sysfs.
* @dev: Device structure for the rgb_led component. This
* device struct is embedded in the rgb_led platform
* device struct.
* @attr: Unused.
* @buf: Buffer that gets returned to user-space.
*
* Return: The number of bytes read.
*/
static ssize_t red_duty_cycle_show(struct device *dev,
    struct device_attribute *attr, char *buf)
{
    u32 red_duty_cycle;

    struct rgb_led_dev *priv = dev_get_drvdata(dev);

    red_duty_cycle = ioread32(priv->red_duty_cycle);

    return scnprintf(buf, PAGE_SIZE, "Red duty cycle = %x\n", red_duty_cycle);
}

/**
* red_duty_cycle_store() - Store the red duty cycle value.
* @dev: Device structure for the rgb_led component. This
* device struct is embedded in the rgb_led platform
* device struct.
* @attr: Unused.
* @buf: Buffer that contains the register value being written.
* @size: The number of bytes being written.
*
* Return: The number of bytes stored.
*/
static ssize_t red_duty_cycle_store(struct device *dev,
struct device_attribute *attr, const char *buf, size_t size)
{
    u32 red_duty_cycle;

    int ret;
    struct rgb_led_dev *priv = dev_get_drvdata(dev);

    // Parse the string we received as a u32
    // See https://elixir.bootlin.com/linux/latest/source/lib/kstrtox.c#L289
    ret = kstrtou32(buf, 0, &red_duty_cycle);
    if (ret < 0) {
    return ret;
    }

    iowrite32(red_duty_cycle, priv->red_duty_cycle);

    // Write was successful, so we return the number of bytes we wrote.
    return size;
}

/**
* green_duty_cycle_show() - Return the green duty cycle value to user-space via sysfs.
* @dev: Device structure for the rgb_led component. This
* device struct is embedded in the rgb_led platform
* device struct.
* @attr: Unused.
* @buf: Buffer that gets returned to user-space.
*
* Return: The number of bytes read.
*/
static ssize_t green_duty_cycle_show(struct device *dev,
    struct device_attribute *attr, char *buf)
{
    u32 green_duty_cycle;

    struct rgb_led_dev *priv = dev_get_drvdata(dev);

    green_duty_cycle = ioread32(priv->green_duty_cycle);

    return scnprintf(buf, PAGE_SIZE, "Green duty cycle = %x\n", green_duty_cycle);
}

/**
* green_duty_cycle_store() - Store the green duty cycle value.
* @dev: Device structure for the rgb_led component. This
* device struct is embedded in the rgb_led platform
* device struct.
* @attr: Unused.
* @buf: Buffer that contains the register value being written.
* @size: The number of bytes being written.
*
* Return: The number of bytes stored.
*/
static ssize_t green_duty_cycle_store(struct device *dev,
struct device_attribute *attr, const char *buf, size_t size)
{
    u32 green_duty_cycle;

    int ret;
    struct rgb_led_dev *priv = dev_get_drvdata(dev);

    // Parse the string we received as a u32
    // See https://elixir.bootlin.com/linux/latest/source/lib/kstrtox.c#L289
    ret = kstrtou32(buf, 0, &green_duty_cycle);
    if (ret < 0) {
    return ret;
    }

    iowrite32(green_duty_cycle, priv->green_duty_cycle);

    // Write was successful, so we return the number of bytes we wrote.
    return size;
}

/**
* blue_duty_cycle_show() - Return the blue duty cycle value to user-space via sysfs.
* @dev: Device structure for the rgb_led component. This
* device struct is embedded in the rgb_led platform
* device struct.
* @attr: Unused.
* @buf: Buffer that gets returned to user-space.
*
* Return: The number of bytes read.
*/
static ssize_t blue_duty_cycle_show(struct device *dev,
    struct device_attribute *attr, char *buf)
{
    u32 blue_duty_cycle;

    struct rgb_led_dev *priv = dev_get_drvdata(dev);

    blue_duty_cycle = ioread32(priv->blue_duty_cycle);

    return scnprintf(buf, PAGE_SIZE, "Blue duty cycle = %x\n", blue_duty_cycle);
}

/**
* blue_duty_cycle_store() - Store the blue duty cycle value.
* @dev: Device structure for the rgb_led component. This
* device struct is embedded in the rgb_led platform
* device struct.
* @attr: Unused.
* @buf: Buffer that contains the register value being written.
* @size: The number of bytes being written.
*
* Return: The number of bytes stored.
*/
static ssize_t blue_duty_cycle_store(struct device *dev,
struct device_attribute *attr, const char *buf, size_t size)
{
    u32 blue_duty_cycle;

    int ret;
    struct rgb_led_dev *priv = dev_get_drvdata(dev);

    // Parse the string we received as a u32
    // See https://elixir.bootlin.com/linux/latest/source/lib/kstrtox.c#L289
    ret = kstrtou32(buf, 0, &blue_duty_cycle);
    if (ret < 0) {
    return ret;
    }

    iowrite32(blue_duty_cycle, priv->blue_duty_cycle);

    // Write was successful, so we return the number of bytes we wrote.
    return size;
}

/**
* period_show() - Return the period value to user-space via sysfs.
* @dev: Device structure for the rgb_led component. This
* device struct is embedded in the rgb_led platform
* device struct.
* @attr: Unused.
* @buf: Buffer that gets returned to user-space.
*
* Return: The number of bytes read.
*/
static ssize_t period_show(struct device *dev,
struct device_attribute *attr, char *buf)
{
    u32 period;
    struct rgb_led_dev *priv = dev_get_drvdata(dev);

    period = ioread32(priv->period);

    return scnprintf(buf, PAGE_SIZE, "Period = %u\n", period);
}

/**
* period_store() - Store the period value.
* @dev: Device structure for the rgb_led component. This
* device struct is embedded in the rgb_led platform
* device struct.
* @attr: Unused.
* @buf: Buffer that contains the period value being written.
* @size: The number of bytes being written.
*
* Return: The number of bytes stored.
*/
static ssize_t period_store(struct device *dev,
struct device_attribute *attr, const char *buf, size_t size)
{
    u32 period;
    int ret;
    struct rgb_led_dev *priv = dev_get_drvdata(dev);

    // Parse the string we received as a u32
    // See https://elixir.bootlin.com/linux/latest/source/lib/kstrtox.c#L289
    ret = kstrtou32(buf, 0, &period);
    if (ret < 0) {
        // kstrtou32 returned an error
        return ret;
    }

    iowrite32(period, priv->period);

    // Write was successful, so we return the number of bytes we wrote.
    return size;
}

// Define sysfs attributes
static DEVICE_ATTR_RW(red_duty_cycle);
static DEVICE_ATTR_RW(green_duty_cycle);
static DEVICE_ATTR_RW(blue_duty_cycle);
static DEVICE_ATTR_RW(period);

// Create an attribute group so the device core can
// export the attributes for us.
static struct attribute *rgb_led_attrs[] = {
    &dev_attr_red_duty_cycle.attr,
    &dev_attr_green_duty_cycle.attr,
    &dev_attr_blue_duty_cycle.attr,
    &dev_attr_period.attr,
    NULL,
};
ATTRIBUTE_GROUPS(rgb_led);

/*
* struct rgb_led_driver - Platform driver struct for the rgb_led driver
* @probe: Function that's called when a device is found
* @remove: Function that's called when a device is removed
* @driver.owner: Which module owns this driver
* @driver.name: Name of the rgb_led driver
* @driver.of_match_table: Device tree match table
*/
static struct platform_driver rgb_led_driver = {
    .probe = rgb_led_probe,
    .remove = rgb_led_remove,
    .driver = {
        .owner = THIS_MODULE,
        .name = "rgb_led",
        .of_match_table = rgb_led_of_match,
        .dev_groups = rgb_led_groups,
    },
};

/*
* We don't need to do anything special in module init/exit.
* This macro automatically handles module init/exit.
*/
module_platform_driver(rgb_led_driver);

MODULE_LICENSE("Dual MIT/GPL");
MODULE_AUTHOR("Seth Howard");
MODULE_DESCRIPTION("rgb_led driver");
MODULE_VERSION("1.0");