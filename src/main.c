/*
 *
 * UDP Test
 *
 */

#include <zephyr.h>
#include <bsd.h>
#include <net/socket.h>
#include <modem/lte_lc.h>
#include <modem/at_cmd.h>
#include <modem/at_notif.h>

#include <drivers/gpio.h>
#include <stdio.h>

#include "cJSON.h"

#define LED_PORT        DT_GPIO_LABEL(DT_ALIAS(led0), gpios)
#define LED1	 DT_GPIO_PIN(DT_ALIAS(led0), gpios)
#define LED2	 DT_GPIO_PIN(DT_ALIAS(led1), gpios)
#define LED3	 DT_GPIO_PIN(DT_ALIAS(led2), gpios)
#define LED4	 DT_GPIO_PIN(DT_ALIAS(led3), gpios)


static int server_socket;
static struct sockaddr_storage server;

// Request product serial number identification +CGSN
static const char at_CGSN[] = "AT+CGSN=1";
// Request manufacturer identification +CGMI
static const char at_CGMI[] = "AT+CGMI";
// Request model identification +CGMM
static const char at_CGMM[] = "AT+CGMM";
// Request revision identification +CGMR
static const char at_CGMR[] = "AT+CGMR";
// Check if a PIN code is needed
static const char at_CPIN[] = "AT+CPIN?";

static const char at_CGPADDR[] = "AT+CGPADDR=0";
static const char at_CGDCONT[] = "AT+CGDCONT?";


#define IMEI_LEN 1024

LOG_MODULE_REGISTER(app, CONFIG_TEST1_LOG_LEVEL);

#if defined(CONFIG_BSD_LIBRARY)

/**@brief Recoverable BSD library error. */
void bsd_recoverable_error_handler(uint32_t err)
{
  printk("bsdlib recoverable error: %u\n", err);
}

/**@brief Irrecoverable BSD library error. */
void bsd_irrecoverable_error_handler(uint32_t err)
{
  printk("bsdlib irrecoverable error: %u\n", err);

  __ASSERT_NO_MSG(false);
}

#endif /* defined(CONFIG_BSD_LIBRARY) */

struct device *led_device;

static void init_led()
{

    led_device = device_get_binding(LED_PORT);

    /* Set LED pin as output */
    gpio_pin_configure(led_device, DT_GPIO_PIN(DT_ALIAS(led0), gpios),
                       GPIO_OUTPUT_ACTIVE |
                       DT_GPIO_FLAGS(DT_ALIAS(led0), gpios));
    gpio_pin_configure(led_device, DT_GPIO_PIN(DT_ALIAS(led1), gpios),
                       GPIO_OUTPUT_ACTIVE |
                       DT_GPIO_FLAGS(DT_ALIAS(led1), gpios));
    gpio_pin_configure(led_device, DT_GPIO_PIN(DT_ALIAS(led2), gpios),
                       GPIO_OUTPUT_ACTIVE |
                       DT_GPIO_FLAGS(DT_ALIAS(led2), gpios));
    gpio_pin_configure(led_device, DT_GPIO_PIN(DT_ALIAS(led3), gpios),
                       GPIO_OUTPUT_ACTIVE |
                       DT_GPIO_FLAGS(DT_ALIAS(led3), gpios));


}


static void led_on(char led)
{
    gpio_pin_set(led_device, led, 1);
}
static void led_off(char led)
{
    gpio_pin_set(led_device, led, 0);

}

static void led_on_off(char led, bool on_off)
{
    if (on_off)
    {
        led_on(led);
    } else {
        led_off(led);
    }
}

int at_command(const char *constcmd) {

    int err;
    enum at_cmd_state at_state;
    char imei_buf[IMEI_LEN];

    err = at_cmd_write(constcmd, imei_buf, sizeof(imei_buf), &at_state);
    if (err) {
        LOG_ERR("at_cmd_write [%s] error:%d, at_state: %d",
                log_strdup(constcmd), err, at_state);

    }
    if (at_state == AT_CMD_OK) {
        LOG_INF("%s OK", log_strdup(constcmd));
    }

    k_sleep(K_MSEC(2000));
    return err;


}


static void init_modem(void)
{
    int err;

    err = at_command(at_CGSN);
    __ASSERT(err == 0, "ERROR: at_command %d %s\n", err, log_strdup(at_CGSN));
    err = at_command(at_CGMI);
    __ASSERT(err == 0, "ERROR: at_command %d %s\n", err, log_strdup(at_CGMI));
    err = at_command(at_CGMM);
    __ASSERT(err == 0, "ERROR: at_command %d %s\n", err, log_strdup(at_CGMM));
    err = at_command(at_CGMR);
    __ASSERT(err == 0, "ERROR: at_command %d %s\n", err, log_strdup(at_CGMR));


    err = lte_lc_init_and_connect();
    __ASSERT(err == 0, "ERROR: LTE link init and connect %d\n", err);

    err = lte_lc_psm_req(false);
    __ASSERT(err == 0, "ERROR: psm %d\n", err);

     err = lte_lc_edrx_req(false);
    __ASSERT(err == 0, "ERROR: edrx %d\n", err);

    LOG_INF("Connected to LTE network");

    err = at_command(at_CPIN);
    __ASSERT(err == 0, "ERROR: at_command %d %s\n", err, log_strdup(at_CPIN));

    err = at_command(at_CGPADDR);
    __ASSERT(err == 0, "ERROR: at_command %d %s\n", err, log_strdup(at_CGPADDR));

    err = at_command(at_CGDCONT);
    __ASSERT(err == 0, "ERROR: at_command %d %s\n", err, log_strdup(at_CGDCONT));
}

static int udp_ip_resolve(void)
{
    struct addrinfo *addrinfo;

    struct addrinfo hints = {
      .ai_family = AF_INET,
      .ai_socktype = SOCK_DGRAM};

    char ipv4_addr[NET_IPV4_ADDR_LEN];

    if (getaddrinfo(CONFIG_SERVER_HOST, NULL, &hints, &addrinfo) != 0)
    {
        LOG_ERR("ERROR: getaddrinfo failed\n");
        return -EIO;
    }

    if (addrinfo == NULL)
    {
        LOG_ERR("ERROR: Address not found\n");
        return -ENOENT;
    }

    struct sockaddr_in *server_ipv4 = ((struct sockaddr_in *)&server);

    server_ipv4->sin_addr.s_addr = ((struct sockaddr_in *)addrinfo->ai_addr)->sin_addr.s_addr;
    server_ipv4->sin_family = AF_INET;
    server_ipv4->sin_port = htons(CONFIG_SERVER_PORT);

    inet_ntop(AF_INET, &server_ipv4->sin_addr.s_addr, ipv4_addr, sizeof(ipv4_addr));
    LOG_INF("Server IPv4 Address %s\n", log_strdup(ipv4_addr));

    freeaddrinfo(addrinfo);

    return 0;
}

int create_udp_socket()
{
    int err;

    struct timeval timeout;

    timeout.tv_sec = 2;
    timeout.tv_usec = 500000;  // 2.5 seconds

    server_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (server_socket < 0)
    {
        LOG_ERR("Failed to create CoAP socket: %d.\n", errno);
        return -errno;
    }

    err = connect(server_socket, (struct sockaddr *)&server,
                sizeof(struct sockaddr_in));
    if (err < 0)
    {
        LOG_ERR("Connect failed : %d\n", errno);
        return -errno;
    }

    setsockopt(server_socket, SOL_SOCKET, SO_RCVTIMEO, (const char *) &timeout, sizeof timeout);

    return 0;
}

bool led_toggle = false;

char* create_json_msg() {



    cJSON *monitor = cJSON_CreateObject();

    cJSON *name = cJSON_CreateString("BSD Test");
    led_toggle = !led_toggle;
    cJSON *led1 = cJSON_CreateBool(led_toggle);
    cJSON *led2 = cJSON_CreateBool(!led_toggle);

    cJSON_AddItemToObject(monitor, "ActionName", name);
    cJSON_AddItemToObject(monitor, "LED1", led1);
    cJSON_AddItemToObject(monitor, "LED2", led2);

    char *string = cJSON_PrintUnformatted(monitor);

    cJSON_Delete(monitor);

    return string;
}

static void action_json_msg(char *msgbuf) {

    cJSON *monitor_json = cJSON_Parse(msgbuf);

    if (monitor_json == NULL)
    {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL)
        {
            LOG_ERR("ERROR: cJSON Parse : %s\n", error_ptr);
            return;
        }
    }

    cJSON *value_name = cJSON_GetObjectItemCaseSensitive(monitor_json, "ActionName");
    if (cJSON_IsString(value_name) && (value_name->valuestring != NULL))
    {
        if (strcmp((value_name->valuestring),"BSD Test") == 0) {
            cJSON *value_led1 = cJSON_GetObjectItemCaseSensitive(monitor_json, "LED1");
            if (cJSON_IsString(value_name) && (value_name->valuestring != NULL)) {
                led_on_off(LED3, value_led1->valueint);
            }
            cJSON *value_led2 = cJSON_GetObjectItemCaseSensitive(monitor_json, "LED2");
            if (cJSON_IsString(value_name) && (value_name->valuestring != NULL)) {
                led_on_off(LED4, value_led2->valueint);
            }
        }
    }

    cJSON_Delete(monitor_json);
}


int send_udp_msg()
{
    char msgbuf[100];
    char *string = create_json_msg();

    int ret = send(server_socket, string, strlen(string), 0);

    LOG_INF("Send packet data [%s] To %s:%d. ret=%d",log_strdup(string), log_strdup(CONFIG_SERVER_HOST), CONFIG_SERVER_PORT, ret);

    int recsize = recv(server_socket, msgbuf, sizeof msgbuf, 0);
    if (recsize > 0) {
        LOG_INF("Received packet size %d data [%s]",recsize, log_strdup(msgbuf));

        action_json_msg(msgbuf);
    } else if (recsize == -1) {
        LOG_INF("Received packet time out");
    }

    return 0;
}


void main(void)
{
    init_led();
    led_off(LED1);
    led_off(LED2);
    led_off(LED3);
    led_off(LED4);

    LOG_INF("BSD Test V1.2.2");
    led_on(LED1);

    LOG_INF("Initializing Modem");
    init_modem();

    int err = udp_ip_resolve();
    __ASSERT(err == 0, "ERROR: udp_ip_resolve");

    create_udp_socket();
    __ASSERT(err == 0, "ERROR: create_udp_socket");

    for (;;) {

        led_on(LED2);

        send_udp_msg();

        led_off(LED2);

        k_sleep(K_MSEC(1000));
    }
}
