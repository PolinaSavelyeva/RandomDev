#include <asm/uaccess.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

#define DEVICE_NAME "randomdev"

static int major_num = 0;

static ssize_t randomdev_read(struct file *flip, char *buffer, size_t length,
                              loff_t *offset) {
  return 0;
}

static ssize_t randomdev_write(struct file *flip, const char *buffer,
                               size_t length, loff_t *offset) {
  return 0;
}

static struct file_operations file_ops = {.read = randomdev_read,
                                          .write = randomdev_write};

static int __init randomdev_init(void) {
  major_num = register_chrdev(0, DEVICE_NAME, &file_ops);

  if (major_num < 0) {
    printk(KERN_ALERT "Registering char device named randomdev failed.");
    return major_num;
  }

  printk(KERN_INFO "I was assigned major number %d. To talk to\n", major_num);
  printk(KERN_INFO "the driver, create a dev file with\n");
  printk(KERN_INFO "'mknod /dev/%s c %d 0'.\n", DEVICE_NAME, major_num);
  printk(KERN_INFO "Try various minor numbers. Try to cat and echo to\n");
  printk(KERN_INFO "the device file.\n");
  printk(KERN_INFO "Remove the device file and module when done.\n");

  return 0;
}

static void __exit randomdev_exit(void) {
  unregister_chrdev(major_num, DEVICE_NAME);
  printk(KERN_INFO "Char device randomdev has been unregistered\n");
}

module_init(randomdev_init);
module_exit(randomdev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Polina Savelyeva");
MODULE_DESCRIPTION("Simple integer generator.");
