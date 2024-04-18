#include <asm/uaccess.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include "ff.h"

#define DEVICE_NAME "randomdev"

static int major_num;

static ssize_t randomdev_read(struct file *flip, char *buffer, size_t length,
                              loff_t *offset) {
  printk(KERN_ALERT "Sorry, this operation isn't supported.\n");
  return -EINVAL;
}

/* Called when a process writes to dev file: echo "hi" > /dev/hello */
static ssize_t randomdev_write(struct file *flip, const char *buffer,
                               size_t length, loff_t *offset) {
  printk(KERN_ALERT "Sorry, this operation isn't supported.\n");
  return -EINVAL;
}

static struct file_operations file_ops = {.read = randomdev_read,
                                          .write = randomdev_write};

static int __init randomdev_init(void) {
  major_num = register_chrdev(0, DEVICE_NAME, &file_ops);

  if (major_num < 0) {
    printk(KERN_ALERT "Registering char device named randomdev failed.");
    return major_num;
  }

  printk(KERN_INFO "I was assigned major number %d.\n", major_num);
  printk(
      KERN_INFO
      "To talk to the driver, create a dev file with 'mknod /dev/%s c %d 0'.\n",
      DEVICE_NAME, major_num);
  printk(KERN_INFO
         "Try to cat and echo to the device file. Remove the device file and "
         "module when done.\n");

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
