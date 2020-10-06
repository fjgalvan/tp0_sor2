//Fernando Javier Galvan  TP0 SOR2, 2/2020
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h> 
#include <linux/ctype.h>
//#include <linux/stdio.h>

int init_module(void);
void cleanup_module(void);
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

void cifrar(char *mensaje, char *destino, int const_N);
void descifrar(char *mensaje, char *destino, int const_N);
int ord(char c);

#define SUCCESS 0
#define DEVICE_NAME "UNGS"
#define BUF_LEN 80

#define LONGITUD_ALFABETO 26
#define INICIO_MAYUSCULAS 65
#define INICIO_MINUSCULAS 97
#define MOD(i, n) (i % n + n) % n // Calcular módulo positivo
const char *aMinusculas = "abcdefghijklmnopqrstuvwxyz";
const char *aMayusculas = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

static int Major;
static int Device_Open = 0;
static char msg[BUF_LEN];
static char *memoria_buffer;
static char endChar= '\0';



static struct file_operations fops = {
	.owner = THIS_MODULE,
	.read = device_read,
    .write = device_write,
    .open = device_open,
    .release = device_release
};

//se ejecuta al cargar un nuevo modulo
int init_module(void)
{
    Major = register_chrdev(0, DEVICE_NAME, &fops);

    if (Major < 0) {
        printk(KERN_ALERT "Registrando char device con %d\n", Major);
        return Major;
    }

    printk(KERN_INFO "Tengo major number %d.Hablarle al driver ", Major);
    printk(KERN_INFO ", crear un dev_file con \n");
    printk(KERN_INFO "sudo rm /dev/%s\n", DEVICE_NAME);
    printk(KERN_INFO "sudo mknod /dev/%s c %d 0\n", DEVICE_NAME, Major);
    printk(KERN_INFO "sudo chmod 666 /dev/%s\n", DEVICE_NAME);
    printk(KERN_INFO "Probá varios minor numbers. Probar cat y echo\n");
    printk(KERN_INFO "al device file.\n");
    printk(KERN_INFO "Eliminar el /dev y el modulo al termina.\n");


    return SUCCESS;
}

//sirve para quitar el modulo
void cleanup_module(void)
{
    /* Liberamos numero mayor */
    unregister_chrdev(Major, DEVICE_NAME);
    printk("Quitando charDev\n");
}

//sirve para abrir el dispositivo
static int device_open(struct inode *inode, struct file *file)
{
    #ifdef DEBUG
    printk(KERN_INFO "device_open(%p)\n", file);
    #endif
   
    if (Device_Open)
        return -EBUSY;
    Device_Open++;
    
    memoria_buffer = msg;
    return SUCCESS;
}

//sirve para abrir el dispositivo (modulo)
static int device_release(struct inode *inode, struct file *filp)
{
    Device_Open--; 
    return SUCCESS;
}

//sirve para leer el dispositivo
static ssize_t device_read(struct file *filp,char *buffer,size_t length,loff_t *offset)
{
    int bytes_read = 0; 
    if (*memoria_buffer == 0) 
        return 0; 
    
    while (length && *memoria_buffer) { 
        put_user(*(memoria_buffer++), buffer++); 
        length--; 
        bytes_read++; 
    } 

    return bytes_read;
}

//sirve para escribir en el char device
static ssize_t device_write(struct file *file, const char *buffer, size_t length, loff_t * offset)
{   
    char mensajeCifrado[BUF_LEN];
    char mensajeDescifrado[BUF_LEN];
    int const_N=1;

    int i;
    #ifdef DEBUG
    printk(KERN_INFO "device_write(%p,%s)", file, buffer);
    #endif
    for (i = 0; i < length && i < BUF_LEN; i++)
        get_user(msg[i], buffer + i);

    msg[length]= endChar;
    //memoria_buffer = msg; //original, sin el cifrar() y descifrar
    cifrar(msg,mensajeCifrado, const_N);
    mensajeCifrado[length]= endChar;
    for (i = 0; i < length && i < BUF_LEN; i++)
        msg[i]=mensajeCifrado[i];
    memoria_buffer=mensajeCifrado;
    descifrar(mensajeCifrado, mensajeDescifrado, const_N);
    //printf("El mensaje descifrado es: %s\n", mensajeDescifrado);
    return length;
}

//Cifrado caesar, sirve para correr todos los caracteres const_N lugares
void cifrar(char *mensaje, char *destino, int const_N) {
  int i = 0;
  while (mensaje[i]) {
    char caracterActual = mensaje[i];
    int posicionOriginal = ord(caracterActual);
    if (!isalpha(caracterActual)) {
      destino[i] = caracterActual;
      i++;
      continue; 
    }
    if (isupper(caracterActual)) {
      destino[i] =
          aMayusculas[(posicionOriginal - INICIO_MAYUSCULAS +
                              const_N) %
                             LONGITUD_ALFABETO];
    } else {
      destino[i] =
          aMinusculas[(posicionOriginal - INICIO_MINUSCULAS +
                              const_N) %
                             LONGITUD_ALFABETO];
    }
    i++;
  }
}

//Descifrado caesar, sirve para correr todos los caracteres const_N lugares para atras
void descifrar(char *mensaje, char *destino, int const_N) {
  int i = 0;
  while (mensaje[i]) {
    char caracterActual = mensaje[i];
    int posicionOriginal = ord(caracterActual);
    if (!isalpha(caracterActual)) {
      destino[i] = caracterActual;
      i++;
      continue; 
    }
    if (isupper(caracterActual)) {
      destino[i] = aMayusculas[MOD(
          posicionOriginal - INICIO_MAYUSCULAS - const_N,
          LONGITUD_ALFABETO)];
    } else {
      destino[i] = aMinusculas[MOD(
          posicionOriginal - INICIO_MINUSCULAS - const_N,
          LONGITUD_ALFABETO)];
    }
    i++;
  }
}
int ord(char c) { return (int)c; }