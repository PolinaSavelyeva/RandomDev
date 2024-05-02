#include <asm/uaccess.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>

#include "ff.h"

#define DEVICE_NAME "randomdev"
#define MAX_INPUT_LENGTH 16

static int major_num;
static ff_elem_t a_vals[MAX_INPUT_LENGTH];
static ff_elem_t x_vals[MAX_INPUT_LENGTH];
static ff_elem_t c;
static int k_order = 0;
static int device_is_opened = 0;	

static int randomdev_open(struct inode *inode, struct file *file)
{
	if (device_is_opened)
		return -EBUSY;

	device_is_opened++;
	try_module_get(THIS_MODULE);

	return 0;
}

static int randomdev_release(struct inode *inode, struct file *file)
{
	device_is_opened--;
	module_put(THIS_MODULE);

	return 0;
}

static ssize_t randomdev_read(struct file *flip, char *buffer, size_t length,
                              loff_t *offset) {
  int i, j, next_int;
  ff_elem_t tmp_mult, tmp_add;
  ff_elem_t next_x = ff_copy(c);

  printk(KERN_DEBUG "Entering: %s\n", __func__);

  printk(KERN_DEBUG "Copying was successfull\n");

  /* Count new x element */
  for (i = 0; i < k_order; i++) {
    printk(KERN_DEBUG "Reading %d byte was successfull\n", i);
    tmp_mult = ff_mult(a_vals[i], x_vals[i]);
    printk(KERN_DEBUG "Mult byte was successfull\n");
    tmp_add = ff_add(next_x, tmp_mult);
    printk(KERN_DEBUG "Add byte was successfull\n");

    ff_elem_free(tmp_mult);
    printk(KERN_DEBUG "Free mult was successfull\n");
    ff_elem_free(next_x);
    printk(KERN_DEBUG "Free next_x was successfull\n");

    next_x = tmp_add;
  }

  kfree(x_vals[0]);

  memmove(x_vals, x_vals + 1, (k_order - 1) * sizeof(ff_elem_t));
  x_vals[k_order - 1] = next_x;

  next_int = ff_2_8_to_uint8(next_x);

  printk(KERN_DEBUG "\n-----------next_x: %u\n", next_int);
  printk(KERN_DEBUG "k_order: %d\n", k_order);

  for (i = 0; i < k_order; i++) {
    printk(KERN_DEBUG "\na_vals num %d:", i);
    for (j = 0; j < 8; j++) printk(KERN_DEBUG "Elem %u", a_vals[i]->coeffs[j]);
  }

  for (i = 0; i < k_order; i++) {
    printk(KERN_DEBUG "\nx_vals num %d:", i);
    for (j = 0; j < 8; j++) printk(KERN_DEBUG "Elem %u", x_vals[i]->coeffs[j]);
  }

  printk(KERN_DEBUG "\nc:\n");
  for (j = 0; j < 8; j++) printk(KERN_DEBUG "Elem %u", c->coeffs[j]);

  if (copy_to_user(buffer, &next_int, 1)) {
    printk(KERN_ERR
           "Memory in kernel space could not be copied to user space.\n");

    return -EFAULT;
  }

  return 1;
}

static ssize_t randomdev_write(struct file *flip, const char *buffer,
                               size_t length, loff_t *offset) {
  // TODO free when not NULL came or put vars to global states

  int i, j;
  uint8_t *data = kmalloc(length, GFP_KERNEL);

  printk(KERN_DEBUG "Entering: %s\n", __func__);

  for (i = 0; i < k_order; i++) {
    printk(KERN_DEBUG "\nx_vals adresses %p:\n", x_vals[i]);
  }
  for (i = 0; i < k_order; i++) {
    printk(KERN_DEBUG "\na_vals adresses %p:\n", a_vals[i]);
  }

  if (!data) {
    printk(KERN_ERR "Memory allocation failed.\n");

    return -EFAULT;
  }

/*   if (!access_ok(buffer, length)) {
    printk(KERN_ERR "Pointer to a block of memory in user space is invalid.\n");
    kfree(data);

    return -EFAULT;
  } */

  if (copy_from_user(data, buffer, length)) {
    printk(KERN_ERR
           "Memory in user space could not be copied to kernel space.\n");
    kfree(data);

    return -EFAULT;
  }

  k_order = data[0];

  if (length != 2 * k_order + 2) {
    printk(KERN_ALERT
           "Unexpected length of input parameters or incorrect "
           "input data type.\n");
    printk(KERN_ALERT "Expected %d integer elements, but given: %lu\n",
           2 * k_order + 2, length);
    kfree(data);

    return -EFAULT;
  }

  for (i = 0; i < k_order; i++) {
    a_vals[i] = ff_2_8_init_elem(data[1 + i]);
    x_vals[i] = ff_2_8_init_elem(data[1 + k_order + i]);
  }

  c = ff_2_8_init_elem(data[1 + 2 * k_order]);

  kfree(data);

  printk(KERN_DEBUG "k_order: %d\n", k_order);

  for (i = 0; i < k_order; i++) {
    printk(KERN_DEBUG "\na_vals num %d:", i);
    for (j = 0; j < 8; j++) printk(KERN_DEBUG "Elem %u", a_vals[i]->coeffs[j]);
  }

  for (i = 0; i < k_order; i++) {
    printk(KERN_DEBUG "\nx_vals num %d:", i);
    for (j = 0; j < 8; j++) printk(KERN_DEBUG "Elem %u", x_vals[i]->coeffs[j]);
  }

  printk(KERN_DEBUG "\nc:\n");
  for (j = 0; j < 8; j++) printk(KERN_DEBUG "Elem %u", c->coeffs[j]);

  return length;
}

static struct file_operations file_ops = {
  .owner = THIS_MODULE, /* This field is used to prevent the module from being unloaded while its operations are in use */
  .open = randomdev_open,
  .release = randomdev_release,
  .read = randomdev_read,
  .write = randomdev_write
  };

static int __init randomdev_init(void) {
  major_num = register_chrdev(0, DEVICE_NAME, &file_ops);

  if (major_num < 0) {
    printk(KERN_ERR "Registering char device named randomdev failed.");

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
  // TODO free allocated vars
  unregister_chrdev(major_num, DEVICE_NAME);
  printk(KERN_INFO "Char device randomdev has been unregistered\n");
}

module_init(randomdev_init);
module_exit(randomdev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Polina Savelyeva");
MODULE_DESCRIPTION("Simple integer generator.");
