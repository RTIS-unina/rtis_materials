# Kernel Installation debugging
When you install a new kernel version on your laptop and you have issues, first you need to understand the problem.
Install QEMU and try to reproduce the error in QEMU in order to capture and analyze kernel logs.

```
sudo qemu-system-x86_64 \
  -m 2048 \
  -kernel /boot/vmlinuz-6.8.0-custom \
  -initrd /boot/initrd.img-6.8.0-custom \
  -append "root=UUID=... ro earlyprintk=ttyS0 console=ttyS0 loglevel=7" \
  -serial file:mon:stdio \
  -nographic
```
where root=UUID=... (you can find it by using blkid)

e.g.,
```
sudo qemu-system-x86_64   -m 4048   -kernel /boot/vmlinuz-5.19.17   -initrd /boot/initrd.img-5.19.17   -append "root=UUID=87cd0949-959b-4f34-a6ef-c8e6b1b7eb81 ro earlyprintk=ttyS0 console=ttyS0 loglevel=7"   -serial mon:stdio   -nographic
```
# Out of Memory
dracut is a modular tool used to generate initramfs images (initial RAM file systems) for Linux systems.
An initramfs is a temporary root filesystem that is loaded into memory during the early stages of boot. It provides the necessary environment to initialize hardware, load kernel modules, and mount the real root filesystem.

Try to reduce the size of you initrd. 	Try to omit network drivers and tools from the initramfs to reduce size and complexity
```
sudo apt install dracut-core
dracut --force --kver 5.19.17 --no-hostonly --omit network
```

Try to add drivers (e.g., to access and load the root fs fromthe ssd)
```
sudo dracut --force --kver 5.19.17 \
  --no-hostonly \
  --add-drivers "virtio_blk virtio_pci ahci nvme ext4"
```


