#include "qemu/osdep.h"
#include "qapi/error.h"
#include "qemu/iov.h"
#include "qemu/module.h"
#include "qemu/timer.h"
#include "qemu/module.h"
#include "qemu/timer.h"
#include "hw/virtio/virtio.h"
#include "hw/qdev-properties.h"
#include "hw/virtio/virtio-hyro.h"
#include "system/runstate.h"
#include "qom/object_interfaces.h"
#include "trace.h"

#define QUEUE_SIZE 128

static void handle_input(VirtIODevice *vdev, VirtQueue *vq)
{
    VirtQueueElement *elem = virtqueue_pop(vq, sizeof(VirtQueueElement));
    if (!elem) {
        printf("[QEMU] No item in queue\n");
        return;
    }

    char *message = elem->out_sg[0].iov_base;
    printf("[QEMU] Received message from guest: %s\n", message);
    
    snprintf(message, 64, "Hello from QEMU!");
    virtqueue_push(vq, elem, sizeof("Hello from QEMU!"));
    virtio_notify(vdev, vq);
    g_free(elem);  // Free the allocated memory
}
static void virtio_hyro_device_realize(DeviceState *dev, Error **errp)
{
    VirtIODevice *vdev = VIRTIO_DEVICE(dev);
    VirtIOHYRO *vhyro = VIRTIO_HYRO(dev);

    virtio_init(vdev, VIRTIO_ID_HYRO, 0);
    vhyro->vq = virtio_add_queue(vdev, QUEUE_SIZE, handle_input);
}

static void virtio_hyro_device_unrealize(DeviceState *dev)
{
    VirtIODevice *vdev = VIRTIO_DEVICE(dev);
    virtio_del_queue(vdev, 0);
    virtio_cleanup(vdev);
}

static const VMStateDescription vmstate_virtio_hyro = {
    .name = "virtio-hyro",
    .minimum_version_id = 1,
    .version_id = 1,
    .fields = (const VMStateField[]) {
        VMSTATE_VIRTIO_DEVICE,
        VMSTATE_END_OF_LIST()
    },
};

//THIS IS MANDATORY
static uint64_t get_features(VirtIODevice *vdev, uint64_t f, Error **errp)
{
    return f;
}

static void virtio_hyro_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    VirtioDeviceClass *vdc = VIRTIO_DEVICE_CLASS(klass);

    dc->vmsd = &vmstate_virtio_hyro;
    set_bit(DEVICE_CATEGORY_MISC, dc->categories);
    vdc->realize = virtio_hyro_device_realize;
    vdc->get_features = get_features;
    vdc->unrealize = virtio_hyro_device_unrealize;
}

static const TypeInfo virtio_hyro_info = {
    .name = TYPE_VIRTIO_HYRO,
    .parent = TYPE_VIRTIO_DEVICE,
    .instance_size = sizeof(VirtIOHYRO),
    .class_init = virtio_hyro_class_init,
};

static void virtio_register_types(void)
{
    type_register_static(&virtio_hyro_info);
}

type_init(virtio_register_types)
