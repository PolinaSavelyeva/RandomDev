#include <asm/uaccess.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>

#include "ff.h"

#define DEVICE_NAME "randomdev"

static uint8_t major_num;
static uint32_t k_order;
static ff_elem_t *a_vals, *x_vals, c;

static void free_all(void) {
  uint32_t i;
  unsigned int size;

  if (k_order) {
    for (i = 0, size = sizeof(ff_elem_t); i < k_order; i++) {
      ff_elem_free(a_vals[i]);
      ff_elem_free(x_vals[i]);
    }
    kfree(a_vals);
    kfree(x_vals);

    ff_elem_free(c);
  }
}

static void *xkmalloc(size_t n) {
  void *res = kmalloc(n, GFP_KERNEL);
  if (!res)
    return NULL;
  return res;
}

static ssize_t randomdev_read(struct file *flip, char *buffer, size_t length,
                              loff_t *offset) {
  uint32_t i;
  uint8_t next_num;
  ff_elem_t tmp_mult, tmp_add, next_x = ff_copy(c);

  for (i = 0; i < k_order; i++) {
    tmp_mult = ff_mult(a_vals[i], x_vals[i]);
    tmp_add = ff_add(next_x, tmp_mult);

    ff_elem_free(tmp_mult);
    ff_elem_free(next_x);

    next_x = tmp_add;
  }

  kfree(x_vals[0]);

  memmove(x_vals, x_vals + 1, (k_order - 1) * sizeof(ff_elem_t));
  x_vals[k_order - 1] = next_x;

  next_num = ff_2_8_to_uint8(next_x);

  printk(KERN_DEBUG "Next num: %u\n", next_num);

  if (put_user(next_num, buffer)) {
    printk(KERN_ERR
           "Memory in kernel space could not be copied to user space.\n");
    return -EFAULT;
  }

  return 1;
}

static ssize_t randomdev_write(struct file *flip, const char *buffer,
                               size_t length, loff_t *offset) {
  uint32_t i;
  uint8_t *data = kmalloc(length, GFP_KERNEL);

  free_all();

  if (!data) {
    printk(KERN_ERR "Memory allocation failed.\n");

    return -EFAULT;
  }

  if (copy_from_user(data, buffer, length)) {
    printk(KERN_ERR
           "Memory in user space could not be copied to kernel space.\n");
    kfree(data);

    return -EFAULT;
  }

  k_order = data[0];

  if (length != 2 * k_order + 2) {
    printk(KERN_ALERT
           "Unexpected length of input parameters or incorrect input data "
           "type.\n");
    printk(KERN_ALERT "Expected %d integer elements, but given: %lu\n",
           2 * k_order + 2, length);
    kfree(data);

    return -EFAULT;
  }

  a_vals = xkmalloc(sizeof(ff_elem_t) * k_order);
  x_vals = xkmalloc(sizeof(ff_elem_t) * k_order);

  for (i = 0; i < k_order; i++) {
    a_vals[i] = ff_2_8_init_elem(data[1 + i]);
    x_vals[i] = ff_2_8_init_elem(data[1 + k_order + i]);
  }

  c = ff_2_8_init_elem(data[1 + 2 * k_order]);

  kfree(data);

  return length;
}

static struct file_operations file_ops = {.read = randomdev_read,
                                          .write = randomdev_write};

static int __init randomdev_init(void) {
  k_order = 0;

  major_num = register_chrdev(0, DEVICE_NAME,
                              &file_ops); /* Providing 0 as major number is
                                             assigning a new free major */

  if (major_num < 0) {
    printk(KERN_ERR "Registering char device named randomdev failed.");

    return major_num;
  }

  printk(KERN_INFO "I was assigned major number %d.\n", major_num);
  printk(KERN_INFO "To talk to the driver, create a dev file with 'mknod "
                   "/dev/%s c %d 0'.\n",
         DEVICE_NAME, major_num);
  printk(KERN_INFO
         "Try to printf and xxd to the device file. Remove the device file and "
         "module when done.\n");

  return 0;
}

static void __exit randomdev_exit(void) {
  free_all();

  unregister_chrdev(major_num, DEVICE_NAME);
  printk(KERN_INFO "Char device randomdev has been unregistered\n");
}

module_init(randomdev_init);
module_exit(randomdev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Polina Savelyeva");
MODULE_DESCRIPTION("Simple integer generator.");
