#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>
#include <linux/ktime.h>
#include <linux/limits.h>
#include <linux/sched.h>
#include <linux/pci.h>

#define USB_INTEL_XUSB2PR	0xD0
#define XUSB2PR_VALUE		0x01D9

static char func_name[NAME_MAX] = "usb_enable_intel_xhci_ports";
module_param_string(func, func_name, NAME_MAX, S_IRUGO);
MODULE_PARM_DESC(func, "route all ports to EHCI instead of xHCI");

static int entry_handler(struct kretprobe_instance *ri, struct pt_regs *regs)
{
	return 0;
}

static void set_xusb2pr(void)
{
	struct pci_dev *xhci_pdev;
	u32 ports_available;

	xhci_pdev = pci_get_subsys(PCI_VENDOR_ID_INTEL, 0x9c31, PCI_ANY_ID, PCI_ANY_ID, NULL);

	pci_read_config_dword(xhci_pdev, USB_INTEL_XUSB2PR, &ports_available);
	pci_write_config_dword(xhci_pdev, USB_INTEL_XUSB2PR, cpu_to_le32(XUSB2PR_VALUE));
	printk(KERN_INFO "old = 0x%04x, new = 0x%04x\n", ports_available, XUSB2PR_VALUE);
}

static int ret_handler(struct kretprobe_instance *ri, struct pt_regs *regs)
{
	set_xusb2pr();
	return 0;
}

static struct kretprobe my_kretprobe = {
	.handler		= ret_handler,
	.entry_handler		= entry_handler,
	/* Probe up to 20 instances concurrently. */
	.maxactive		= 20,
};

static int __init kretprobe_init(void)
{
	int ret;

	my_kretprobe.kp.symbol_name = func_name;

	ret = register_kretprobe(&my_kretprobe);
	if (ret < 0) {
		printk(KERN_INFO "register_kretprobe failed, returned %d\n",
				ret);
		return -1;
	}

	set_xusb2pr();
	printk(KERN_INFO "Planted return probe at %s: %p\n",
			my_kretprobe.kp.symbol_name, my_kretprobe.kp.addr);
	return 0;
}

static void __exit kretprobe_exit(void)
{
	unregister_kretprobe(&my_kretprobe);

	printk(KERN_INFO "kretprobe at %p unregistered\n",
			my_kretprobe.kp.addr);

	/* nmissed > 0 suggests that maxactive was set too low. */
	printk(KERN_INFO "Missed probing %d instances of %s\n",
		my_kretprobe.nmissed, my_kretprobe.kp.symbol_name);

}

module_init(kretprobe_init)
module_exit(kretprobe_exit)
MODULE_LICENSE("GPL");
