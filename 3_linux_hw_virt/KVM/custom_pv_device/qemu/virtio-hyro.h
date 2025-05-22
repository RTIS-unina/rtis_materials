#ifndef QEMU_VIRTIO_HYRO_H
#define QEMU_VIRTIO_HYRO_H

#include "hw/virtio/virtio.h"
#include "qom/object.h"
#include "standard-headers/linux/virtio_ids.h"
#include "standard-headers/linux/virtio_config.h"


#define TYPE_VIRTIO_HYRO "virtio-hyro-device"
OBJECT_DECLARE_SIMPLE_TYPE(VirtIOHYRO, VIRTIO_HYRO)
#define VIRTIO_HYRO_GET_PARENT_CLASS(obj) \
        OBJECT_GET_PARENT_CLASS(obj, TYPE_VIRTIO_HYRO)

struct VirtIOHYRO {
    VirtIODevice parent_obj;
    VirtQueue *vq;
};

#endif
