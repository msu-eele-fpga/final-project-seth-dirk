#include <linux/module.h>                   // Basic kernel module definitions
#include <linux/platform_device.h>          // Platfrom driver/device definitions
#include <linux/mod_devicetable.h>          // of_device_id and MODULE_DEVICE_TABLE
#include <linux/io.h>                       // iowrite32/ioread32 functions
#include <linux/mutex.h>                    // mutex definitions
#include <linux/miscdevice.h>               // miscdevice definitions
#include <linux/types.h>                    // data types
#include <linux/fs.h>                       // copy_to_user
#include <linux/kstrtox.h>                  // kstrtou32

#define OUTPUT_OFFSET         0x00            // 0 byte offset for the rotary encoder state output register
#define ENABLE_OFFSET       0x04            // 4 byte offset for the enable button register
#define SPAN 16                                 // Span of the components memory space

/**
* struct rotary_dev - Private rotary encoder device struct.
* @output: Address of the rotary encoder state output register
* @enable: Address of the enable register
* @miscdev: miscdevice used to create a character device
* @lock: mutex used to prevent concurrent writes to memory
*
* An rotary_dev  struct gets created for each rotary encoder component.
*/
struct rotary_dev {
    void __iomem *base_addr;
    void __iomem *output;
    void __iomem *enable;
    struct miscdevice miscdev;
    struct mutex lock;
};

/*
* rotary_read() - Read method for the rotary char device
* @file: Pointer to the char device file struct.
* @buf: User-space buffer to read the value into.
* @count: The number of bytes being requested.
* @offset: The byte offset in the file being read from.
*
* Return: On success, the number of bytes written is returned and the
* offset @offset is advanced by this number. On error, a negative error
* value is returned.
*/
static ssize_t rotary_read(struct file *file, char __user *buf,
    size_t count, loff_t *offset)
{
    size_t ret;
    u32 val;

    /*
    * Get the device's private data from the file struct's private_data
    * field. The private_data field is equal to the miscdev field in the
    * rotary_dev struct. container_of returns the
    * rotary_dev struct that contains the miscdev in private_data.
    */
    struct rotary_dev *priv = container_of(file->private_data,
                                struct rotary_dev, miscdev);

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
        pr_warn("rotary_read: unaligned access\n");
        return -EFAULT;
    }

    val = ioread32(priv->base_addr + *offset);

    // Copy the value to userspace.
    ret = copy_to_user(buf, &val, sizeof(val));
    if (ret == sizeof(val)) {
        pr_warn("rotary_read: nothing copied\n");
        return -EFAULT;
    }

    // Increment the file offset by the number of bytes we read.
    *offset = *offset + sizeof(val);

    return sizeof(val);
}

/**
* rotary_write() - Write method for the rotary char device even though you should never write to this device lol I don't know what I'm doing
* @file: Pointer to the char device file struct.
* @buf: User-space buffer to read the value from.
* @count: The number of bytes being written.
* @offset: The byte offset in the file being written to.
*
* Return: On success, the number of bytes written is returned and the
* offset @offset is advanced by this number. On error, a negative error
* value is returned.
*/
static ssize_t rotary_write(struct file *file, const char __user *buf,
    size_t count, loff_t *offset)
{
    size_t ret;
    u32 val;

    struct rotary_dev *priv = container_of(file->private_data,
                                struct rotary_dev, miscdev);

    if (*offset < 0) {
        return -EINVAL;
    }
    if (*offset >= SPAN) {
        return 0;
    }
    if ((*offset % 0x4) != 0) {
        pr_warn("rotary_write: unaligned access\n");
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
        pr_warn("rotary_write: nothing copied from user space\n");
        ret = -EFAULT;
    }

    mutex_unlock(&priv->lock);
    return ret;
}

/**
* rotary_fops - File operations supported by the
* rotary driver
* @owner: The rotary driver owns the file operations; this
* ensures that the driver can't be removed while the
* character device is still in use.
* @read: The read function.
* @write: The write function.
* @llseek: We use the kernel's default_llseek() function; this allows
* users to change what position they are writing/reading to/from.
*/
static const struct file_operations rotary_fops = {
    .owner = THIS_MODULE,
    .read = rotary_read,
    .write = rotary_write,
    .llseek = default_llseek,
};

/**
* rotary_probe() - Initialize device when a match is found
* @pdev: Platform device structure associated with our rotary encoder device;
* pdev is automatically created by the driver core based upon our
* rotary device tree node.
*
* When a device that is compatible with this rgb control driver is found, the
* driver's probe function is called. This probe function gets called by the
* kernel when an rgb_led device is found in the device tree.
*/
static int rotary_probe(struct platform_device *pdev)
{
    struct rotary_dev *priv;
    size_t ret;

    /*
    * Allocate kernel memory for the rotary device and set it to 0.
    * GFP_KERNEL specifies that we are allocating normal kernel RAM;
    * see the kmalloc documentation for more info. The allocated memory
    * is automatically freed when the device is removed.
    */
    priv = devm_kzalloc(&pdev->dev, sizeof(struct rotary_dev),
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
    priv->output = priv->base_addr + OUTPUT_OFFSET;
    priv->enable = priv->base_addr + ENABLE_OFFSET;

    // Initialize the misc device parameters
    priv->miscdev.minor = MISC_DYNAMIC_MINOR;
    priv->miscdev.name = "rotary";
    priv->miscdev.fops = &rotary_fops;
    priv->miscdev.parent = &pdev->dev;

    // Register the misc device; this creates a char dev at /dev/rotary
    ret = misc_register(&priv->miscdev);
    if (ret) {
        pr_err("Failed to register misc device");
        return ret;
    }

    /* Attach the rotary ecoder's private data to the platform device's struct.
    * This is so we can access our state container in the other functions.
    */
    platform_set_drvdata(pdev, priv);

    pr_info("rotary_probe successful\n");

    return 0;
}

/**
*rotary_remove() - Remove an rotary encoder device.
* @pdev: Platform device structure associated with our rotary device.
*
* This function is called when an rotary encoder device is removed or
* the driver is removed.
*/
static int rotary_remove(struct platform_device *pdev)
{
    // Get the  rotary encoder's private data from the platform device.
    struct rotary_dev *priv = platform_get_drvdata(pdev);

    // Deregister the misc device and remove the /dev/rotary file.
    misc_deregister(&priv->miscdev);

    pr_info("rotary_remove successful\n");

    return 0;
}

/*
* Define the compatible property used for matching devices to this driver,
* then add our device id structure to the kernel's device table. For a device
* to be matched with this driver, its device tree node must use the same
* compatible string as defined here.
*/
static const struct of_device_id rotary_of_match[] = {
    { .compatible = "Kaiser,rotary", },
    { }
};
MODULE_DEVICE_TABLE(of, rotary_of_match);

/**
* output_show() - Return the rotary encoder output state value to user-space via sysfs.
* @dev: Device structure for the rotary component. This
* device struct is embedded in the rotary platform
* device struct.
* @attr: Unused.
* @buf: Buffer that gets returned to user-space.
*
* Return: The number of bytes read.
*/
static ssize_t output_show(struct device *dev,
    struct device_attribute *attr, char *buf)
{
    u32 output;

    struct rotary_dev *priv = dev_get_drvdata(dev);

    output = ioread32(priv->output);

    return scnprintf(buf, PAGE_SIZE, "Output = %x\n", output);
}

/**
* output_store() - while I shouldnt ever write to the output register, I can't figure out
*		  how to get the sysfs code to work without it, and I can't be bothered with trying to put more time into this silly project
* @dev: device structre for rotary component.
* @attr: unused
* @buf: buffer that contains value
* @size: number of bytes being written
* 
* Return: number of bytes stored
*/ 
static ssize_t output_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
return 0;
}

/**
* enable_show() - Return the enable value to user-space via sysfs.
* @dev: Device structure for the rotary component. This
* device struct is embedded in the rotary platform
* device struct.
* @attr: Unused.
* @buf: Buffer that gets returned to user-space.
*
* Return: The number of bytes read.
*/
static ssize_t enable_show(struct device *dev,
    struct device_attribute *attr, char *buf)
{
    u32 enable;

    struct rotary_dev *priv = dev_get_drvdata(dev);

    enable = ioread32(priv->enable);

    return scnprintf(buf, PAGE_SIZE, "enable = %x\n", enable);
}

/*
* enable_store() - again, this is only here to get the sysfs to compile. 
*		  I know there should be a way better way to do this, but I just don't care
* @dev: device structure for component
* @attr: unused
* @buf: buffer that contains value being written
* @size: number of bytes
*
* Return: the number of bytes stored.
*/
static ssize_t enable_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	return 0;
}

// Define sysfs attributes
static DEVICE_ATTR_RW(output);
static DEVICE_ATTR_RW(enable);

// Create an attribute group so the device core can
// export the attributes for us.
static struct attribute *rotary_attrs[] = {
    &dev_attr_output.attr,
    &dev_attr_enable.attr,
    NULL,
};
ATTRIBUTE_GROUPS(rotary);

/*
* struct rotary_driver - Platform driver struct for the rotary driver
* @probe: Function that's called when a device is found
* @remove: Function that's called when a device is removed
* @driver.owner: Which module owns this driver
* @driver.name: Name of the rotary driver
* @driver.of_match_table: Device tree match table
*/
static struct platform_driver rotary_driver = {
    .probe = rotary_probe,
    .remove = rotary_remove,
    .driver = {
        .owner = THIS_MODULE,
        .name = "rotary",
        .of_match_table = rotary_of_match,
        .dev_groups = rotary_groups,
    },
};

/*
* We don't need to do anything special in module init/exit.
* This macro automatically handles module init/exit.
*/
module_platform_driver(rotary_driver);

MODULE_LICENSE("Dual MIT/GPL");
MODULE_AUTHOR("Dirk Kaiser");
MODULE_DESCRIPTION("rotary driver");
MODULE_VERSION("1.0");
