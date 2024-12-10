#include <linux/module.h>                   // Basic kernel module definitions
#include <linux/platform_device.h>          // Platfrom driver/device definitions
#include <linux/mod_devicetable.h>          // of_device_id and MODULE_DEVICE_TABLE
#include <linux/io.h>                       // iowrite32/ioread32 functions
#include <linux/mutex.h>                    // mutex definitions
#include <linux/miscdevice.h>               // miscdevice definitions
#include <linux/types.h>                    // data types
#include <linux/fs.h>                       // copy_to_user
#include <linux/kstrtox.h>                  // kstrtou32

#define VOLUME_OFFSET   0x00            // 0 byte offset for the volume register
#define PITCH_OFFSET    0x04            // 4 byte offset for the base pitch register
#define SPAN 16                         // Span of the components memory space
/**
* struct buzzer_dev - Private buzzer controller device struct.
* @volume: Address of the volume register
* @pitch: Address of the pitch register
* @miscdev: miscdevice used to create a character device
* @lock: mutex used to prevent concurrent writes to memory
*
* An buzzer_led struct gets created for each buzzer controller component.
*/
struct buzzer_dev {
    void __iomem *base_addr;
    void __iomem *volume;
    void __iomem *pitch;
    struct miscdevice miscdev;
    struct mutex lock;
};

/**
* buzzer_read() - Read method for the buzzer char device
* @file: Pointer to the char device file struct.
* @buf: User-space buffer to read the value into.
* @count: The number of bytes being requested.
* @offset: The byte offset in the file being read from.
*
* Return: On success, the number of bytes written is returned and the
* offset @offset is advanced by this number. On error, a negative error
* value is returned.
*/
static ssize_t buzzer_read(struct file *file, char __user *buf,
    size_t count, loff_t *offset)
{
    size_t ret;
    u32 val;

    /*
    * Get the device's private data from the file struct's private_data
    * field. The private_data field is equal to the miscdev field in the
    * buzzer_dev struct. container_of returns the
    * buzzer_dev struct that contains the miscdev in private_data.
    */
    struct buzzer_dev *priv = container_of(file->private_data,
                                struct buzzer_dev, miscdev);

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
        pr_warn("buzzer_read: unaligned access\n");
        return -EFAULT;
    }

    val = ioread32(priv->base_addr + *offset);

    // Copy the value to userspace.
    ret = copy_to_user(buf, &val, sizeof(val));
    if (ret == sizeof(val)) {
        pr_warn("buzzer_read: nothing copied\n");
        return -EFAULT;
    }

    // Increment the file offset by the number of bytes we read.
    *offset = *offset + sizeof(val);

    return sizeof(val);
}

/**
* buzzer_write() - Write method for the buzzer char device
* @file: Pointer to the char device file struct.
* @buf: User-space buffer to read the value from.
* @count: The number of bytes being written.
* @offset: The byte offset in the file being written to.
*
* Return: On success, the number of bytes written is returned and the
* offset @offset is advanced by this number. On error, a negative error
* value is returned.
*/
static ssize_t buzzer_write(struct file *file, const char __user *buf,
    size_t count, loff_t *offset)
{
    size_t ret;
    u32 val;

    struct buzzer_dev *priv = container_of(file->private_data,
                                struct buzzer_dev, miscdev);

    if (*offset < 0) {
        return -EINVAL;
    }
    if (*offset >= SPAN) {
        return 0;
    }
    if ((*offset % 0x4) != 0) {
        pr_warn("buzzer_write: unaligned access\n");
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
        pr_warn("buzzer_write: nothing copied from user space\n");
        ret = -EFAULT;
    }

    mutex_unlock(&priv->lock);
    return ret;
}

/**
* buzzer_fops - File operations supported by the
* buzzer driver
* @owner: The buzzer driver owns the file operations; this
* ensures that the driver can't be removed while the
* character device is still in use.
* @read: The read function.
* @write: The write function.
* @llseek: We use the kernel's default_llseek() function; this allows
* users to change what position they are writing/reading to/from.
*/
static const struct file_operations buzzer_fops = {
    .owner = THIS_MODULE,
    .read = buzzer_read,
    .write = buzzer_write,
    .llseek = default_llseek,
};

/**
* buzzer_probe() - Initialize device when a match is found
* @pdev: Platform device structure associated with our buzzer control device;
* pdev is automatically created by the driver core based upon our
* buzzer control device tree node.
*
* When a device that is compatible with this buzzer control driver is found, the
* driver's probe function is called. This probe function gets called by the
* kernel when an buzzer device is found in the device tree.
*/
static int buzzer_probe(struct platform_device *pdev)
{
    struct buzzer_dev *priv;
    size_t ret;

    /*
    * Allocate kernel memory for the buzzer control device and set it to 0.
    * GFP_KERNEL specifies that we are allocating normal kernel RAM;
    * see the kmalloc documentation for more info. The allocated memory
    * is automatically freed when the device is removed.
    */
    priv = devm_kzalloc(&pdev->dev, sizeof(struct buzzer_dev),
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
    priv->volume = priv->base_addr + VOLUME_OFFSET;
    priv->pitch = priv->base_addr + PITCH_OFFSET;

    // Set the pitch to 262 Hz (middle C) and volume to 0 to begin
    iowrite32(0x0, priv->volume);
    iowrite32(0x0106, priv->pitch);

    // Initialize the misc device parameters
    priv->miscdev.minor = MISC_DYNAMIC_MINOR;
    priv->miscdev.name = "buzzer";
    priv->miscdev.fops = &buzzer_fops;
    priv->miscdev.parent = &pdev->dev;

    // Register the misc device; this creates a char dev at /dev/buzzer
    ret = misc_register(&priv->miscdev);
    if (ret) {
        pr_err("Failed to register misc device");
        return ret;
    }

    /* Attach the buzzer controller's private data to the platform device's struct.
    * This is so we can access our state container in the other functions.
    */
    platform_set_drvdata(pdev, priv);

    pr_info("buzzer_probe successful\n");

    return 0;
}

/**
*buzzer_remove() - Remove an buzzer control device.
* @pdev: Platform device structure associated with our buzzer control device.
*
* This function is called when an buzzer control device is removed or
* the driver is removed.
*/
static int buzzer_remove(struct platform_device *pdev)
{
    // Get the buzzer control's private data from the platform device.
    struct buzzer_dev *priv = platform_get_drvdata(pdev);

    // Deregister the misc device and remove the /dev/buzzer file.
    misc_deregister(&priv->miscdev);

    pr_info("buzzer_remove successful\n");

    return 0;
}

/*
* Define the compatible property used for matching devices to this driver,
* then add our device id structure to the kernel's device table. For a device
* to be matched with this driver, its device tree node must use the same
* compatible string as defined here.
*/
static const struct of_device_id buzzer_of_match[] = {
    { .compatible = "Howard,buzzer", },
    { }
};
MODULE_DEVICE_TABLE(of, buzzer_of_match);

/**
* volume_show() - Return the volume value to user-space via sysfs.
* @dev: Device structure for the buzzer component. This
* device struct is embedded in the buzzer platform
* device struct.
* @attr: Unused.
* @buf: Buffer that gets returned to user-space.
*
* Return: The number of bytes read.
*/
static ssize_t volume_show(struct device *dev,
    struct device_attribute *attr, char *buf)
{
    u32 volume;

    struct buzzer_dev *priv = dev_get_drvdata(dev);

    volume = ioread32(priv->volume);

    return scnprintf(buf, PAGE_SIZE, "Volume = %x\n", volume);
}

/**
* volume_store() - Store the volume value.
* @dev: Device structure for the buzzer component. This
* device struct is embedded in the buzzer platform
* device struct.
* @attr: Unused.
* @buf: Buffer that contains the register value being written.
* @size: The number of bytes being written.
*
* Return: The number of bytes stored.
*/
static ssize_t volume_store(struct device *dev,
struct device_attribute *attr, const char *buf, size_t size)
{
    u32 volume;

    int ret;
    struct buzzer_dev *priv = dev_get_drvdata(dev);

    // Parse the string we received as a u32
    // See https://elixir.bootlin.com/linux/latest/source/lib/kstrtox.c#L289
    ret = kstrtou32(buf, 0, &volume);
    if (ret < 0) {
    return ret;
    }

    iowrite32(volume, priv->volume);

    // Write was successful, so we return the number of bytes we wrote.
    return size;
}

/**
* pitch_show() - Return the pitch value to user-space via sysfs.
* @dev: Device structure for the buzzer component. This
* device struct is embedded in the buzzer platform
* device struct.
* @attr: Unused.
* @buf: Buffer that gets returned to user-space.
*
* Return: The number of bytes read.
*/
static ssize_t pitch_show(struct device *dev,
    struct device_attribute *attr, char *buf)
{
    u32 pitch;

    struct buzzer_dev *priv = dev_get_drvdata(dev);

    pitch = ioread32(priv->pitch);

    return scnprintf(buf, PAGE_SIZE, "Pitch = %x\n", pitch);
}

/**
* pitch_store() - Store the pitch value.
* @dev: Device structure for the buzzer component. This
* device struct is embedded in the buzzer platform
* device struct.
* @attr: Unused.
* @buf: Buffer that contains the register value being written.
* @size: The number of bytes being written.
*
* Return: The number of bytes stored.
*/
static ssize_t pitch_store(struct device *dev,
struct device_attribute *attr, const char *buf, size_t size)
{
    u32 pitch;

    int ret;
    struct buzzer_dev *priv = dev_get_drvdata(dev);

    // Parse the string we received as a u32
    // See https://elixir.bootlin.com/linux/latest/source/lib/kstrtox.c#L289
    ret = kstrtou32(buf, 0, &pitch);
    if (ret < 0) {
    return ret;
    }

    iowrite32(pitch, priv->pitch);

    // Write was successful, so we return the number of bytes we wrote.
    return size;
}

// Define sysfs attributes
static DEVICE_ATTR_RW(volume);
static DEVICE_ATTR_RW(pitch);

// Create an attribute group so the device core can
// export the attributes for us.
static struct attribute *buzzer_attrs[] = {
    &dev_attr_volume.attr,
    &dev_attr_pitch.attr,
    NULL,
};
ATTRIBUTE_GROUPS(buzzer);

/*
* struct buzzer_driver - Platform driver struct for the buzzer driver
* @probe: Function that's called when a device is found
* @remove: Function that's called when a device is removed
* @driver.owner: Which module owns this driver
* @driver.name: Name of the buzzer driver
* @driver.of_match_table: Device tree match table
*/
static struct platform_driver buzzer_driver = {
    .probe = buzzer_probe,
    .remove = buzzer_remove,
    .driver = {
        .owner = THIS_MODULE,
        .name = "buzzer",
        .of_match_table = buzzer_of_match,
        .dev_groups = buzzer_groups,
    },
};

/*
* We don't need to do anything special in module init/exit.
* This macro automatically handles module init/exit.
*/
module_platform_driver(buzzer_driver);

MODULE_LICENSE("Dual MIT/GPL");
MODULE_AUTHOR("Seth Howard");
MODULE_DESCRIPTION("buzzer driver");
MODULE_VERSION("1.0");