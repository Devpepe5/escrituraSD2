#include <string.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include <sys/unistd.h>
#include <sys/stat.h>
#include "driver/sdspi_host.h"



#define PIN_NUM_MISO 2
#define PIN_NUM_MOSI 15
#define PIN_NUM_CLK  14
#define PIN_NUM_CS   13

#define PUNTO_MONTAJE "/sdcard" //nombre de la SD
#define MOUNT_POINT (PUNTO_MONTAJE)
#define EXAMPLE_MAX_CHAR_SIZE 64

static esp_err_t  escribirArchivo(const char *path, char *data){
    FILE *f = fopen(path, "w");
    if (f == NULL) {
        printf("Failed to open file for writing");
        return ESP_FAIL;
    }
    fprintf(f, data);
    fclose(f);
    return ESP_OK;
}

static esp_err_t  leerArchivo (const char *path){
    printf("Reading file %s", path);
    FILE *f = fopen(path, "r");
    if (f == NULL) {
        printf("Failed to open file for reading");
        return ESP_FAIL;
    }
    char line[EXAMPLE_MAX_CHAR_SIZE];
    fgets(line, sizeof(line), f);
    fclose(f);

    // strip newline
    char *pos = strchr(line, '\n');
    if (pos) {
        *pos = '\0';
    }
    printf("Read from file: '%s'", line);
    return ESP_OK;

}

void app_main(void)
{
    esp_err_t ret;
    sdspi_dev_handle_t SDcardhandle;
    // Options for mounting the filesystem.
    // If format_if_mount_failed is set to true, SD card will be partitioned and
    // formatted in case when mounting fails.
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
#ifdef CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED
        .format_if_mount_failed = true,
#else
        .format_if_mount_failed = false,
#endif // EXAMPLE_FORMAT_IF_MOUNT_FAILED
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };
    sdmmc_card_t *card;
    const char mount_point[] = MOUNT_POINT;
    printf("Initializing SD card\n");

    // Use settings defined above to initialize SD card and mount FAT filesystem.
    // Note: esp_vfs_fat_sdmmc/sdspi_mount is all-in-one convenience functions.
    // Please check its source code and implement error recovery when developing
    // production applications.
    printf("Using SPI peripheral\n");


    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
   // Firstly, use the macro SDSPI_DEVICE_CONFIG_DEFAULT to initialize a structure sdspi_device_config_t, which is used to initialize an SD SPI device
    
    
    /*  
    slot_config.gpio_cd =  ; 
    slot_config.gpio_wp =  ;
    slot_config.gpio_int=  ;
     */

    


        // By default, SD card frequency is initialized to SDMMC_FREQ_DEFAULT (20MHz)
    // For setting a specific frequency, use host.max_freq_khz (range 400kHz - 20MHz for SDSPI)
    // Example: for fixed frequency of 10MHz, use host.max_freq_khz = 10000;
    //Then use SDSPI_HOST_DEFAULT macro to initialize a sdmmc_host_t structure

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();

    spi_bus_config_t bus_cfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    //modify the host an pins 

    slot_config.gpio_cs = PIN_NUM_CS;
    //modify hosid and pins
    slot_config.host_id = host.slot;


    //Then call sdspi_host_init_device to initialize the SD SPI device and attach to its bus.
    //Modify the slot parameter of the structure to the SD SPI device spi_handle just returned from sdspi_host_init_device
    spi_bus_initialize(host.slot, &bus_cfg, SDSPI_DEFAULT_DMA);
    //sdspi_host_init_device(&slot_config, &SDcardhandle);

    //Call sdmmc_card_init with the sdmmc_host_t to probe and initialize the SD card.
    //sdmmc_card_init(host,card);

    /*
    ret = spi_bus_initialize(host.slot, &bus_cfg, SDSPI_DEFAULT_DMA);
    if (ret != ESP_OK) {
        printf("Failed to initialize bus.");
        return;
    }

*/

    printf("Mounting filesystem\n");
    esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);
/*
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            printf("Failed to mount filesystem. "
                     "If you want the card to be formatted, set the CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
        } else {
            printf("Failed to initialize the card (%s). "
                     "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
        }
        return;
    }*/
    printf("Filesystem mounted");

    // Card has been initialized, print its properties
    sdmmc_card_print_info(stdout, card);

    // Use POSIX and C standard library functions to work with files.

    // First create a file.
    const char *file_hello = PUNTO_MONTAJE"/hello.txt";
    char data[EXAMPLE_MAX_CHAR_SIZE];
    snprintf(data, EXAMPLE_MAX_CHAR_SIZE, "%s %s!\n", "Hello", card->cid.name);
    ret = escribirArchivo(file_hello, data);
    if (ret != ESP_OK) {
        return;
    }

    const char *file_foo = PUNTO_MONTAJE"/foo.txt";

    // Check if destination file exists before renaming
    struct stat st;
    if (stat(file_foo, &st) == 0) {
        // Delete it if it exists
        unlink(file_foo);
    }

    // Rename original file
    printf("Renaming file %s to %s", file_hello, file_foo);
    if (rename(file_hello, file_foo) != 0) {
        printf("Rename failed");
    }

    ret = leerArchivo(file_foo);
    if (ret != ESP_OK) {
        return;
    }

    // Format FATFS
/*
     ret = esp_vfs_fat_sdcard_format(mount_point, card);
        if (ret != ESP_OK) {
        printf("Failed to format FATFS (%s)", esp_err_to_name(ret));
        return;
    }
    */

    if (stat(file_foo, &st) == 0) {
        printf("file still exists");
        return;
    } else {
        printf("file doesnt exist, format done");
    }

    const char *file_nihao = PUNTO_MONTAJE"/nihao.txt";
    memset(data, 0, EXAMPLE_MAX_CHAR_SIZE);
    snprintf(data, EXAMPLE_MAX_CHAR_SIZE, "%s %s!\n", "Nihao", card->cid.name);
    ret = escribirArchivo(file_nihao, data);
    if (ret != ESP_OK) {
        return;
    }

    //Open file for reading
    ret = leerArchivo(file_nihao);
    if (ret != ESP_OK) {
        return;
    }

    // All done, unmount partition and disable SPI peripheral
    esp_vfs_fat_sdcard_unmount(mount_point, card);
    printf("Card unmounted");

    //deinitialize the bus after all devices are removed
    spi_bus_free(host.slot);
}