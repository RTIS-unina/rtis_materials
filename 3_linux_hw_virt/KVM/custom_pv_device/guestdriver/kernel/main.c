#include <linux/module.h>
#include <linux/virtio.h>
#include <linux/virtio_config.h>
#include <linux/slab.h>
#include <linux/scatterlist.h>

#define VIRTIO_ID_HYRO 42  // Deve corrispondere all'ID nel backend
#define BUFFER_SIZE 64


static inline long hyro_hypercall_stop_execution(void) {
    register long rax asm("rax") = 99;  // Assegna 98 a RAX
    asm volatile (
        "vmcall"  // Esegue l'hypercall (Intel VMX)
        : "+r" (rax) // Output (rax può essere modificato)
        :           // Nessun input oltre RAX
        : "rbx", "rcx", "rdx", "memory" // Clobbered registers
    );
    return rax;  // Restituisce il valore di RAX dopo la chiamata
}

static inline long hyro_hypercall_send_msg(void *message) {
    register long rax asm("rax") = 100;  // Numero dell'hypercall
    register void *rbx asm("rbx") = message;  // Indirizzo del messaggio

    asm volatile (
        "vmcall"
        : "+r"(rax) // Output: rax può essere modificato
        : "r"(rbx)  // Input: il valore di rbx
        : "rcx", "rdx", "memory" // Clobbered registers
    );

    return rax;  // Restituisce il valore di RAX dopo la chiamata
}

static inline void hyro_hypercall_printk(const char *msg) {
    register uint64_t rax asm("rax") = 102;  // HYRO_PRINTK
    register uint64_t rbx asm("rbx") = (uint64_t)msg;  // Puntatore al buffer
    register uint64_t rcx asm("rcx") = strlen(msg);  // Lunghezza del messaggio

    asm volatile("vmcall"
                 : "+r"(rax)
                 : "r"(rbx), "r"(rcx)
                 : "memory");
                 
}




struct hyro_dev {
    struct virtio_device *vdev;
    struct virtqueue *vq;
};

/**
 * Callback invocata quando il backend invia dati.
 */
static void virtio_hyro_recv_done(struct virtqueue *vq)
{
    struct hyro_dev *hdev = vq->vdev->priv;
    struct virtio_device *vdev = hdev->vdev;
    void *buf;
    unsigned int len;

    pr_info("virtio-hyro: Receiving data from virtqueue\n");

    buf = virtqueue_get_buf(vq, &len);
    if (!buf) {
        pr_err("virtio-hyro: No buffer received\n");
        return;
    }

    pr_info("virtio-hyro: Message from backend: %s\n", (char *)buf);
    

    kfree(buf);
}

/**
 * Invia un messaggio alla coda VirtIO.
 */
static void virtio_hyro_send_msg(struct hyro_dev *hdev, const char *msg)
{
    struct scatterlist sg;
    char *buf;

    buf = kmalloc(BUFFER_SIZE, GFP_KERNEL);
    if (!buf)
        return;

    snprintf(buf, BUFFER_SIZE, "%s", msg);
    sg_init_one(&sg, buf, BUFFER_SIZE);

    if (virtqueue_add_outbuf(hdev->vq, &sg, 1, buf, GFP_KERNEL)) {
        pr_err("virtio-hyro: Failed to add buffer to queue\n");
        kfree(buf);
        return;
    }

    virtqueue_kick(hdev->vq);
    pr_info("virtio-hyro: Sent message to backend: %s\n", msg);
}

/**
 * Trova la virtqueue associata al dispositivo.
 */
static struct virtqueue *find_vq(struct virtio_device *vdev)
{
    struct virtqueue *vq;
    int err;
    static vq_callback_t *callbacks[] = { virtio_hyro_recv_done };
    static const char *names[] = { "hyro-vq" };

    pr_info("virtio-hyro: Searching for virtqueue\n");

    err = virtio_find_vqs(vdev, 1, &vq, callbacks, names, NULL);
    if (err) {
        pr_err("virtio-hyro: Failed to find virtqueue\n");
        return NULL;
    }

    return vq;
}

/**
 * Viene chiamata quando il kernel rileva un dispositivo VirtIO compatibile.
 */
static int virtio_hyro_probe(struct virtio_device *vdev)
{
    struct hyro_dev *hdev;

    pr_info("virtio-hyro: Probing device\n");

    hdev = kzalloc(sizeof(*hdev), GFP_KERNEL);
    if (!hdev)
        return -ENOMEM;

    hdev->vdev = vdev;
    vdev->priv = hdev;

    hdev->vq = find_vq(vdev);
    if (!hdev->vq) {
        kfree(hdev);
        return -ENXIO;
    }

    virtio_device_ready(vdev);
    pr_info("virtio-hyro: Device initialized\n");
    hyro_hypercall_stop_execution();
    // Esempio: Invia un messaggio al backend
    virtio_hyro_send_msg(hdev, "Hello from Kernel!");

    return 0;
}

/**
 * Rimuove il dispositivo quando viene disconnesso.
 */
static void virtio_hyro_remove(struct virtio_device *vdev)
{
    struct hyro_dev *hdev = vdev->priv;

    pr_info("virtio-hyro: Removing device\n");

    vdev->config->reset(vdev);
    vdev->config->del_vqs(vdev);
    kfree(hdev);
}

/**
 * Definizione degli ID supportati.
 */
static struct virtio_device_id id_table[] = {
    { VIRTIO_ID_HYRO, VIRTIO_DEV_ANY_ID },
    { 0 }
};

/**
 * Struttura del driver VirtIO.
 */
static struct virtio_driver virtio_hyro_driver = {
    .driver.name = "virtio_hyro",
    .driver.owner = THIS_MODULE,
    .id_table = id_table,
    .probe = virtio_hyro_probe,
    .remove = virtio_hyro_remove,
};

/**
 * Inizializzazione del modulo.
 */
static int __init virtio_hyro_init(void)
{
    return register_virtio_driver(&virtio_hyro_driver);
}

/**
 * Cleanup del modulo.
 */
static void __exit virtio_hyro_exit(void)
{
    pr_info("virtio-hyro: Exiting driver\n");
    unregister_virtio_driver(&virtio_hyro_driver);
}

module_init(virtio_hyro_init);
module_exit(virtio_hyro_exit);

MODULE_DEVICE_TABLE(virtio, id_table);
MODULE_DESCRIPTION("Virtio-Hyro Frontend Driver (With TX/RX)");
MODULE_LICENSE("GPL");

