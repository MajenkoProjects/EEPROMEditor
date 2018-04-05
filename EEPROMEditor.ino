#include <DTWI.h>
#include <EEPROM24_DTWI.h>
#include <CLI.h>
#include <DFATFS.h>
#include <USB_MSD.h>
#include <FLASHVOL.h>

DTWI1 dtwi;
EEPROM24 eeprom(dtwi, 0x50, 256);
uint8_t buffer[256];

#define VOLSIZ 131072
//const uint32_t __attribute__((aligned(4096), section(".romdata"))) flashVol[VOLSIZ/4] = {0};
const uint32_t __attribute__((aligned(4096))) flashVol[VOLSIZ/4] = {0};
FLASHVOL vol(flashVol, VOLSIZ/512);

USBFS usbDevice;
USBManager USB(usbDevice, 0xf055, 0x3276, "Majenko Technologies", "EEPROM Editor");
CDCACM USBSerial;
USB_MSD msd(vol);

CLI_COMMAND(probe) {
    (void)argc; (void)argv; // Prevent "unused" warnings
    dev->println("Probing devices...");
    dtwi.beginMaster();
    int nDevices = 0;
    for(int address = 8; address < 127; address++ ) 
    {
        // The DTWIscanner uses the return value of
        // the DTWI0.stopMaster() to see if
        // a device did acknowledge to the address.
        dtwi.startMasterRead(address);
        delay(10);
        if(!dtwi.stopMaster())
        {
            dev->printf("I2C device found at address 0x%02x\r\n", address);
            nDevices++;
        }
    }
    if (nDevices == 0) dev->println("No I2C devices found");
    dtwi.endMaster();
    return 0;
}

CLI_COMMAND(readit) {
    (void)argc; (void)argv; // Prevent "unused" warnings
    eeprom.begin();
    char data[17] = {0};
    uint8_t dpos = 0;
    int b = 0;
    for (int addr = 0; addr < 256; addr++) {
        if (addr % 16 == 0) {
            dev->printf(" %s\r\n%02x: ", data, addr);
            dpos = 0;
        }
        uint8_t v = eeprom.read(addr);
        buffer[b++] = v;
        dev->printf("%02x ", v);
        data[dpos++] = (v >= 32 && v <= 126) ? v : '.';
        data[dpos] = 0;        
    }
    dev->printf(" %s\r\n", data);
    eeprom.end();
    return 0;   
}

CLI_COMMAND(writeit) {
    (void)argc; (void)argv; (void)dev; // Prevent "unused" warnings
    eeprom.begin();
    for (int i = 0; i < 256; i++) {
        eeprom.write(i, buffer[i]);
    }
    return 0;
}

CLI_COMMAND(printit) {
    (void)argc; (void)argv; // Prevent "unused" warnings
    char data[17] = {0};
    uint8_t dpos = 0;
    for (int addr = 0; addr < 256; addr++) {
        if (addr % 16 == 0) {
            dev->printf(" %s\r\n%02x: ", data, addr);
            dpos = 0;
        }
        uint8_t v = buffer[addr];
        dev->printf("%02x ", v);
        data[dpos++] = (v >= 32 && v <= 126) ? v : '.';
        data[dpos] = 0;        
    }
    dev->printf(" %s\r\n", data);
    return 0;       
}

CLI_COMMAND(loadit) {
    DFILE f;
    uint32_t r;
    
    if (argc != 2) {
        dev->println("Usage: load <filename>");
        return 10;
    }
    if (DFATFS::fsmount(vol, DFATFS::szFatFsVols[0], 1) != FR_OK) {
        dev->println("Error mounting flash disk.");
        return 10;
    }

    if (f.fsopen(argv[1], FA_READ) != FR_OK) {
        dev->printf("Error opening %s for reading.\r\n", argv[1]);
        DFATFS::fsunmount(DFATFS::szFatFsVols[0]);   
        return 10;    
    }
    f.fsread(buffer, 256, &r);
    f.fsclose();
    DFATFS::fsunmount(DFATFS::szFatFsVols[0]);   
    return 0;
}

CLI_COMMAND(delit) {
    DFILE f;
    
    if (argc != 2) {
        dev->println("Usage: rm <filename>");
        return 10;
    }
    if (DFATFS::fsmount(vol, DFATFS::szFatFsVols[0], 1) != FR_OK) {
        dev->println("Error mounting flash disk.");
        return 10;
    }

    if (DFATFS::fsunlink(argv[1]) != FR_OK) {
        dev->printf("Error deleting %s.\r\n", argv[1]);
        DFATFS::fsunmount(DFATFS::szFatFsVols[0]);   
        return 10;    
    }
    DFATFS::fsunmount(DFATFS::szFatFsVols[0]);   
    return 0;
}

CLI_COMMAND(saveit) {
    DFILE f;
    uint32_t r;
    
    if (argc != 2) {
        dev->println("Usage: save <filename>");
        return 10;
    }
    if (DFATFS::fsmount(vol, DFATFS::szFatFsVols[0], 1) != FR_OK) {
        dev->println("Error mounting flash disk.");
        return 10;
    }

    if (f.fsopen(argv[1], FA_CREATE_ALWAYS | FA_WRITE | FA_READ) != FR_OK) {
        dev->printf("Error opening %s for writing.\r\n", argv[1]);
        DFATFS::fsunmount(DFATFS::szFatFsVols[0]);   
        return 10;    
    }
    f.fswrite(buffer, 256, &r);
    f.fsclose();
    DFATFS::fsunmount(DFATFS::szFatFsVols[0]);
    return 0;
}

int parseParameter(const char *str) {
    if ((str[0] == '^') && (strlen(str) > 1)) {
        return (int)str[1];
    }

    if ((strncasecmp(str, "0x", 2) == 0) && (strlen(str) > 2)){
        return strtoul(str+2, NULL, 16);
    }

    if ((strncasecmp(str, "0b", 2) == 0) && (strlen(str) > 2)){
        return strtoul(str+2, NULL, 2);
    }

    if (str[0] == '0') {
        return strtoul(str, NULL, 8);        
    }

    if ((str[0] >= '1') && (str[0] <= '9')) {
        return strtoul(str, NULL, 10);
    }
    return -1;
}

CLI_COMMAND(setit) {
    if (argc != 3) {
        dev->println("Usage: set <address> <value>");
        return 10;
    }

    int addr = parseParameter(argv[1]);
    int val = parseParameter(argv[2]);

    if (addr < 0) {
        dev->printf("Error: %s is not understandable\r\n", argv[1]);
        return 10;
    }

    if (val < 0) {
        dev->printf("Error: %s is not understandable\r\n", argv[2]);
        return 10;
    }

    if (addr > 255 || addr < 0) {
        dev->println("Error: Address out of range");
        return 10;
    }

    if (val > 255 || val < 0) {
        dev->println("Error: Value out of range");
        return 10;
    }

    buffer[addr] = val;
    return 0;
}

CLI_COMMAND(listit) {    
    (void)argc; (void)argv; // Prevent "unused" warnings
    int files = 0;
    int total = 0;
    
    if (DFATFS::fsmount(vol, DFATFS::szFatFsVols[0], 1) != FR_OK) {
        dev->println("Error mounting flash disk.");
        return 10;
    }

    FRESULT fr;

    fr = DDIRINFO::fsopendir("/");
    if (fr != FR_OK) {
        dev->printf("Unable to open /: %d\r\n", fr);
        DFATFS::fsunmount(DFATFS::szFatFsVols[0]);
        return 10;
    }

    fr = DDIRINFO::fsreaddir();
    while (fr == FR_OK) {
        if (strlen(DDIRINFO::fsget8Dot3Filename()) == 0) {
            break;
        }
        files++;
        total += DDIRINFO::fsgetFileSize();
        dev->printf("%10d %s\r\n", DDIRINFO::fsgetFileSize(), DDIRINFO::fsget8Dot3Filename());
        fr = DDIRINFO::fsreaddir();
    }

    DDIRINFO::fsclosedir();

    if (files == 0) {
        dev->println("No files found.");
    } else if (files == 1) {
        dev->printf("%d file, %d bytes.\r\n", files, total);
    } else {
        dev->printf("%d files, %d bytes.\r\n", files, total);
    }
    DFATFS::fsunmount(DFATFS::szFatFsVols[0]);
    return 0;
}

void setup() {

    srand(analogRead(0) ^ analogRead(0) ^ analogRead(0) ^ analogRead(0));
    if (DFATFS::fsmount(vol, DFATFS::szFatFsVols[0], 1) != FR_OK) {
        DFATFS::fsmkfs(vol, 0xDEADBEEF);
        DFATFS::fsmount(vol, DFATFS::szFatFsVols[0], 1);
        DFILE f;
        f.fsopen("allzero.dat", FA_CREATE_ALWAYS | FA_WRITE | FA_READ);
        for (int i = 0; i < 256; i++) {
            f.write(0x00);
        }
        f.fsclose();
        f.fsopen("allff.dat", FA_CREATE_ALWAYS | FA_WRITE | FA_READ);
        for (int i = 0; i < 256; i++) {
            f.write(0xff);
        }
        f.fsclose();
    }

    DFATFS::fsunmount(DFATFS::szFatFsVols[0]);                

    USB.addDevice(msd);
    USB.addDevice(USBSerial);
    USB.begin();
    
    CLI.setDefaultPrompt("> ");
    CLI.addClient(USBSerial);

    CLI.addCommand("scan", probe);
    CLI.addCommand("read", readit);
    CLI.addCommand("print", printit);
    CLI.addCommand("load", loadit);
    CLI.addCommand("save", saveit);
    CLI.addCommand("write", writeit);
    CLI.addCommand("set", setit);
    CLI.addCommand("ls", listit);
    CLI.addCommand("rm", delit);
}

void loop() {
    CLI.process();
}
