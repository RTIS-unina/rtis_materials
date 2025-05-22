/*
 * Virtio hyro PCI Bindings
 *
 * Copyright 2012 Red Hat, Inc.
 * Copyright 2012 Amit Shah <amit.shah@redhat.com>
 *
 * This work is licensed under the terms of the GNU GPL, version 2 or
 * (at your option) any later version.  See the COPYING file in the
 * top-level directory.
 */

#include "qemu/osdep.h"

#include "hw/virtio/virtio-pci.h"
#include "hw/virtio/virtio-hyro.h"
#include "hw/qdev-properties.h"
#include "qapi/error.h"
#include "qemu/module.h"
#include "qom/object.h"

typedef struct VirtIOHyroPCI VirtIOHyroPCI;

/*
 * virtio-hyro-pci: This extends VirtioPCIProxy.
 */
#define TYPE_VIRTIO_HYRO_PCI "virtio-hyro-pci-base"
DECLARE_INSTANCE_CHECKER(VirtIOHyroPCI, VIRTIO_HYRO_PCI,
                         TYPE_VIRTIO_HYRO_PCI)

struct VirtIOHyroPCI {
    VirtIOPCIProxy parent_obj;
    VirtIOHYRO vdev;
};

static const Property virtio_hyro_properties[] = {
    DEFINE_PROP_BIT("ioeventfd", VirtIOPCIProxy, flags,
                    VIRTIO_PCI_FLAG_USE_IOEVENTFD_BIT, true),
    DEFINE_PROP_UINT32("vectors", VirtIOPCIProxy, nvectors,
                       DEV_NVECTORS_UNSPECIFIED),
};

static void virtio_hyro_pci_realize(VirtIOPCIProxy *vpci_dev, Error **errp)
{
    VirtIOHyroPCI *vhyro = VIRTIO_HYRO_PCI(vpci_dev);
    DeviceState *vdev = DEVICE(&vhyro->vdev);

    if (vpci_dev->nvectors == DEV_NVECTORS_UNSPECIFIED) {
        vpci_dev->nvectors = 2;
    }

    if (!qdev_realize(vdev, BUS(&vpci_dev->bus), errp)) {
        return;
    }
}

static void virtio_hyro_pci_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    VirtioPCIClass *k = VIRTIO_PCI_CLASS(klass);
    PCIDeviceClass *pcidev_k = PCI_DEVICE_CLASS(klass);

    k->realize = virtio_hyro_pci_realize;
    set_bit(DEVICE_CATEGORY_MISC, dc->categories);

    pcidev_k->vendor_id = PCI_VENDOR_ID_REDHAT_QUMRANET;
    pcidev_k->device_id = PCI_DEVICE_ID_VIRTIO_HYRO;
    pcidev_k->revision = VIRTIO_PCI_ABI_VERSION;
    pcidev_k->class_id = PCI_CLASS_OTHERS;
    device_class_set_props(dc, virtio_hyro_properties);
}

static void virtio_hyro_initfn(Object *obj)
{
    VirtIOHyroPCI *dev = VIRTIO_HYRO_PCI(obj);

    virtio_instance_init_common(obj, &dev->vdev, sizeof(dev->vdev),
                                TYPE_VIRTIO_HYRO);
}

static const VirtioPCIDeviceTypeInfo virtio_hyro_pci_info = {
    .base_name             = TYPE_VIRTIO_HYRO_PCI,
    .generic_name          = "virtio-hyro-pci",
    .transitional_name     = "virtio-hyro-pci-transitional",
    .non_transitional_name = "virtio-hyro-pci-non-transitional",
    .instance_size = sizeof(VirtIOHyroPCI),
    .instance_init = virtio_hyro_initfn,
    .class_init    = virtio_hyro_pci_class_init,
};

static void virtio_hyro_pci_register(void)
{
    virtio_pci_types_register(&virtio_hyro_pci_info);
}

type_init(virtio_hyro_pci_register)
