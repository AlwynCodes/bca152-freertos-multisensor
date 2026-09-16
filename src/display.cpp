#include "display.h"
#include "config.h"

#include "driver/i2c_master.h"
#include "esp_err.h"
#include "esp_log.h"

#include <cstdio>
#include <cstring>

static const char *TAG = "DISPLAY";

static i2c_master_bus_handle_t i2c_bus = nullptr;
static i2c_master_dev_handle_t oled_device = nullptr;

static constexpr uint8_t OLED_WIDTH = 128;
static constexpr uint8_t OLED_PAGES = 8;

static uint8_t framebuffer[OLED_WIDTH * OLED_PAGES];

/*
 * Minimal 5x7 font used by the OLED display.
 * Characters required by the room-monitoring display are included.
 */
static void get_glyph(char character, uint8_t glyph[5])
{
    memset(glyph, 0, 5);

    switch (character) {
        case 'A': glyph[0]=0x7E; glyph[1]=0x11; glyph[2]=0x11; glyph[3]=0x11; glyph[4]=0x7E; break;
        case 'B': glyph[0]=0x7F; glyph[1]=0x49; glyph[2]=0x49; glyph[3]=0x49; glyph[4]=0x36; break;
        case 'C': glyph[0]=0x3E; glyph[1]=0x41; glyph[2]=0x41; glyph[3]=0x41; glyph[4]=0x22; break;
        case 'D': glyph[0]=0x7F; glyph[1]=0x41; glyph[2]=0x41; glyph[3]=0x22; glyph[4]=0x1C; break;
        case 'E': glyph[0]=0x7F; glyph[1]=0x49; glyph[2]=0x49; glyph[3]=0x49; glyph[4]=0x41; break;
        case 'F': glyph[0]=0x7F; glyph[1]=0x09; glyph[2]=0x09; glyph[3]=0x09; glyph[4]=0x01; break;
        case 'G': glyph[0]=0x3E; glyph[1]=0x41; glyph[2]=0x49; glyph[3]=0x49; glyph[4]=0x7A; break;
        case 'H': glyph[0]=0x7F; glyph[1]=0x08; glyph[2]=0x08; glyph[3]=0x08; glyph[4]=0x7F; break;
        case 'I': glyph[0]=0x00; glyph[1]=0x41; glyph[2]=0x7F; glyph[3]=0x41; glyph[4]=0x00; break;
        case 'J': glyph[0]=0x20; glyph[1]=0x40; glyph[2]=0x41; glyph[3]=0x3F; glyph[4]=0x01; break;
        case 'K': glyph[0]=0x7F; glyph[1]=0x08; glyph[2]=0x14; glyph[3]=0x22; glyph[4]=0x41; break;
        case 'L': glyph[0]=0x7F; glyph[1]=0x40; glyph[2]=0x40; glyph[3]=0x40; glyph[4]=0x40; break;
        case 'M': glyph[0]=0x7F; glyph[1]=0x02; glyph[2]=0x0C; glyph[3]=0x02; glyph[4]=0x7F; break;
        case 'N': glyph[0]=0x7F; glyph[1]=0x04; glyph[2]=0x08; glyph[3]=0x10; glyph[4]=0x7F; break;
        case 'O': glyph[0]=0x3E; glyph[1]=0x41; glyph[2]=0x41; glyph[3]=0x41; glyph[4]=0x3E; break;
        case 'P': glyph[0]=0x7F; glyph[1]=0x09; glyph[2]=0x09; glyph[3]=0x09; glyph[4]=0x06; break;
        case 'Q': glyph[0]=0x3E; glyph[1]=0x41; glyph[2]=0x51; glyph[3]=0x21; glyph[4]=0x5E; break;
        case 'R': glyph[0]=0x7F; glyph[1]=0x09; glyph[2]=0x19; glyph[3]=0x29; glyph[4]=0x46; break;
        case 'S': glyph[0]=0x46; glyph[1]=0x49; glyph[2]=0x49; glyph[3]=0x49; glyph[4]=0x31; break;
        case 'T': glyph[0]=0x01; glyph[1]=0x01; glyph[2]=0x7F; glyph[3]=0x01; glyph[4]=0x01; break;
        case 'U': glyph[0]=0x3F; glyph[1]=0x40; glyph[2]=0x40; glyph[3]=0x40; glyph[4]=0x3F; break;
        case 'V': glyph[0]=0x1F; glyph[1]=0x20; glyph[2]=0x40; glyph[3]=0x20; glyph[4]=0x1F; break;
        case 'W': glyph[0]=0x3F; glyph[1]=0x40; glyph[2]=0x38; glyph[3]=0x40; glyph[4]=0x3F; break;
        case 'X': glyph[0]=0x63; glyph[1]=0x14; glyph[2]=0x08; glyph[3]=0x14; glyph[4]=0x63; break;
        case 'Y': glyph[0]=0x07; glyph[1]=0x08; glyph[2]=0x70; glyph[3]=0x08; glyph[4]=0x07; break;
        case 'Z': glyph[0]=0x61; glyph[1]=0x51; glyph[2]=0x49; glyph[3]=0x45; glyph[4]=0x43; break;

        case '0': glyph[0]=0x3E; glyph[1]=0x51; glyph[2]=0x49; glyph[3]=0x45; glyph[4]=0x3E; break;
        case '1': glyph[0]=0x00; glyph[1]=0x42; glyph[2]=0x7F; glyph[3]=0x40; glyph[4]=0x00; break;
        case '2': glyph[0]=0x42; glyph[1]=0x61; glyph[2]=0x51; glyph[3]=0x49; glyph[4]=0x46; break;
        case '3': glyph[0]=0x21; glyph[1]=0x41; glyph[2]=0x45; glyph[3]=0x4B; glyph[4]=0x31; break;
        case '4': glyph[0]=0x18; glyph[1]=0x14; glyph[2]=0x12; glyph[3]=0x7F; glyph[4]=0x10; break;
        case '5': glyph[0]=0x27; glyph[1]=0x45; glyph[2]=0x45; glyph[3]=0x45; glyph[4]=0x39; break;
        case '6': glyph[0]=0x3C; glyph[1]=0x4A; glyph[2]=0x49; glyph[3]=0x49; glyph[4]=0x30; break;
        case '7': glyph[0]=0x01; glyph[1]=0x71; glyph[2]=0x09; glyph[3]=0x05; glyph[4]=0x03; break;
        case '8': glyph[0]=0x36; glyph[1]=0x49; glyph[2]=0x49; glyph[3]=0x49; glyph[4]=0x36; break;
        case '9': glyph[0]=0x06; glyph[1]=0x49; glyph[2]=0x49; glyph[3]=0x29; glyph[4]=0x1E; break;

        case ':': glyph[0]=0x00; glyph[1]=0x36; glyph[2]=0x36; glyph[3]=0x00; glyph[4]=0x00; break;
        case '.': glyph[0]=0x00; glyph[1]=0x60; glyph[2]=0x60; glyph[3]=0x00; glyph[4]=0x00; break;
        case '%': glyph[0]=0x63; glyph[1]=0x13; glyph[2]=0x08; glyph[3]=0x64; glyph[4]=0x63; break;
        case '-': glyph[0]=0x08; glyph[1]=0x08; glyph[2]=0x08; glyph[3]=0x08; glyph[4]=0x08; break;
        case ' ': break;

        default:
            break;
    }
}

static esp_err_t oled_write(const uint8_t *data, size_t length)
{
    if (oled_device == nullptr) {
        return ESP_ERR_INVALID_STATE;
    }

    return i2c_master_transmit(
        oled_device,
        data,
        length,
        100
    );
}

static esp_err_t oled_command(uint8_t command)
{
    const uint8_t packet[2] = {
        0x00,
        command
    };

    return oled_write(packet, sizeof(packet));
}

static esp_err_t oled_data(const uint8_t *data, size_t length)
{
    uint8_t packet[129] = {};

    if (length > 128) {
        return ESP_ERR_INVALID_SIZE;
    }

    packet[0] = 0x40;

    memcpy(
        &packet[1],
        data,
        length
    );

    return oled_write(
        packet,
        length + 1
    );
}

static void oled_clear_buffer(void)
{
    memset(
        framebuffer,
        0,
        sizeof(framebuffer)
    );
}

static void oled_draw_char(
    uint8_t x,
    uint8_t page,
    char character
)
{
    if (page >= OLED_PAGES || x > 122) {
        return;
    }

    uint8_t glyph[5];
    get_glyph(character, glyph);

    for (uint8_t column = 0; column < 5; ++column) {
        framebuffer[
            page * OLED_WIDTH + x + column
        ] = glyph[column];
    }
}

static void oled_draw_text(
    uint8_t x,
    uint8_t page,
    const char *text
)
{
    while (*text != '\0' && x <= 122) {
        oled_draw_char(
            x,
            page,
            *text
        );

        x += 6;
        ++text;
    }
}

static void oled_refresh(void)
{
    for (uint8_t page = 0; page < OLED_PAGES; ++page) {

        oled_command(
            static_cast<uint8_t>(0xB0 | page)
        );

        oled_command(0x00);
        oled_command(0x10);

        oled_data(
            &framebuffer[page * OLED_WIDTH],
            OLED_WIDTH
        );
    }
}

static void oled_init(void)
{
    i2c_master_bus_config_t bus_config = {};

    bus_config.clk_source = I2C_CLK_SRC_DEFAULT;
    bus_config.i2c_port = I2C_NUM_0;

    bus_config.sda_io_num =
        static_cast<gpio_num_t>(OLED_SDA_GPIO);

    bus_config.scl_io_num =
        static_cast<gpio_num_t>(OLED_SCL_GPIO);

    bus_config.glitch_ignore_cnt = 7;

    bus_config.flags.enable_internal_pullup = true;

    ESP_ERROR_CHECK(
        i2c_new_master_bus(
            &bus_config,
            &i2c_bus
        )
    );

    i2c_device_config_t device_config = {};

    device_config.dev_addr_length =
        I2C_ADDR_BIT_LEN_7;

    device_config.device_address =
        OLED_I2C_ADDRESS;

    device_config.scl_speed_hz =
        400000;

    ESP_ERROR_CHECK(
        i2c_master_bus_add_device(
            i2c_bus,
            &device_config,
            &oled_device
        )
    );

    const uint8_t commands[] = {
        0xAE,
        0xD5,
        0x80,
        0xA8,
        0x3F,
        0xD3,
        0x00,
        0x40,
        0x8D,
        0x14,
        0x20,
        0x00,
        0xA1,
        0xC8,
        0xDA,
        0x12,
        0x81,
        0x7F,
        0xD9,
        0xF1,
        0xDB,
        0x40,
        0xA4,
        0xA6,
        0xAF
    };

    for (uint8_t command : commands) {
        ESP_ERROR_CHECK(
            oled_command(command)
        );
    }

    ESP_LOGI(
        TAG,
        "OLED initialized"
    );
}

static void oled_clear(void)
{
    oled_clear_buffer();
    oled_refresh();
}

DisplayMode nextDisplayMode(DisplayMode mode)
{
    if (mode == DISPLAY_MOTION) {
        return DISPLAY_TEMPERATURE;
    }

    return static_cast<DisplayMode>(
        static_cast<int>(mode) + 1
    );
}

DisplayMode previousDisplayMode(DisplayMode mode)
{
    if (mode == DISPLAY_TEMPERATURE) {
        return DISPLAY_MOTION;
    }

    return static_cast<DisplayMode>(
        static_cast<int>(mode) - 1
    );
}

void init_display(void)
{
    oled_init();
    oled_clear();
}

void display_sensor_data(
    const SensorData *data,
    DisplayMode mode
)
{
    if (data == nullptr) {
        return;
    }

    char label[32] = {};
    char value[32] = {};

    switch (mode) {

        case DISPLAY_TEMPERATURE:
            snprintf(
                label,
                sizeof(label),
                "TEMPERATURE"
            );

            snprintf(
                value,
                sizeof(value),
                "%.2f C",
                data->temperature
            );
            break;

        case DISPLAY_HUMIDITY:
            snprintf(
                label,
                sizeof(label),
                "HUMIDITY"
            );

            snprintf(
                value,
                sizeof(value),
                "%.2f %%",
                data->humidity
            );
            break;

        case DISPLAY_LIGHT:
            snprintf(
                label,
                sizeof(label),
                "LIGHT"
            );

            snprintf(
                value,
                sizeof(value),
                "%d %%",
                data->lightLevel
            );
            break;

        case DISPLAY_MOTION:
            snprintf(
                label,
                sizeof(label),
                "MOTION"
            );

            snprintf(
                value,
                sizeof(value),
                "%s",
                data->motionDetected
                    ? "DETECTED"
                    : "NONE"
            );
            break;
    }

    oled_clear_buffer();

    oled_draw_text(
        0,
        0,
        label
    );

    oled_draw_text(
        0,
        2,
        value
    );

    oled_refresh();

    ESP_LOGI(
        TAG,
        "%s: %s",
        label,
        value
    );
}