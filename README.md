# Objectives

### Read the Objectives

By the end of this module, you should be able to:

* Create a C program that sends temperature data to the gateway via MQTT and also display it on LCD

# Creating an MQTT Service to Publish the Temperature Sensor Data

## Plug in the Grove shield, temperature sensor and the LCD

In the Sensors and Actuators lab, we connected the temperature sensor (Analog) and LCD display (I2C) to your sensor hub device. We wrote C code to measure the temperature in Celsius using upm library, convert it to Fahrenheit, then display it on the LCD.

Your project should start looking like the picture on the right.

1. Install the Grove Base Shield onto the sensor computer device (For this lab, this will be either a Up2 Board or a NUCi7).
2. Connect **Grove Temperature Sensor** to analog pin **A0** of the Grove Base Shield.
3. Connect **Grove LCD** display to one of the **I2C** pins.

## Start a new C program to add MQTT feature to our earlier temperature module

1. Create a new project directory.
2. Create a C file named temperature_mqtt.c.

This temperature project will be used in all the later labs to supply a steady stream of data.

### Write the C program that will send temperature data over MQTT

Update **temperature_mqtt.c** with following changes

1.  Include the following C headers in your program. Here MQTTClient.h is the header file of Paho MQTT C client library implemenation of MQTT protocol

    ```c
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include "MQTTClient.h"

    #include "jhd1313m1.h"
    #include "temperature.h"
    #include "upm.h"
    #include "upm_utilities.h"
    #include "signal.h"
    #include "string.h"
    ```

2.  Define the MQTT parameters we will use to connect using MQTT protocol. Here ADDRESS is our localhost on port 1883 and TOPIC is sensors/temperature/data on which we will publish the temperature values

    ``` c
    #define ADDRESS     "tcp://localhost:1883"
    #define CLIENTID    "MQTTExample"
    #define TOPIC       "sensors/temperature/data"
    #define QOS         1
    #define TIMEOUT     10000L
    ```

3.  Write the signal handler function to handle termination of the program which will continuously read temperature values, display it on LCD and send it over MQTT topic

    ```c
    void sig_handler(int signo)
    {
        if (signo == SIGINT) {
            printf("closing IO%d nicely\n", iopin);
            running = -1;
        }
    }
    ```

4.  In the main function of the program first connect to MQTT client with the setup parameters. This is MQTT synchronous connection. It is also possible to do an asynchronous connect
	```c
    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;
    int rc;
    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to connect, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }
    ```

5.  Next create the temperature sensor and LCD context
	```c
    jhd1313m1_context lcd = jhd1313m1_init(512, 0x3e, 0x62);
    temperature_context temp = temperature_init(512);
	```
6.  Create a JSON Object for MQTT to publish

```c
    // Create JSON Object for MQTT
    struct timeval time;
    gettimeofday(&time,NULL);
    char snum[2];
    sprintf(snum, "%d", (int) celsius);
    char cnum[13];
    sprintf(cnum, "%d", (int) time.tv_sec);
    char *sensor = "sensor_id";
    char *tempur = "temperature";
    char *value  = "value";
    char *timet   = "timestamp";
    struct json_object *jobj;
    jobj = json_object_new_object();
    json_object_object_add(jobj, sensor, json_object_new_string(tempur));
    json_object_object_add(jobj, value, json_object_new_string(snum));
    json_object_object_add(jobj, timet, json_object_new_string(cnum));
    str_mqtt = json_object_to_json_string(jobj);

```

7.  Finally in the while loop read the temperature value, convert it to fahrenheit and then store the value in two strings. These strings are then sent to LCD for display and also as payload topic to MQTT publish command. Below is how it will publish this MQTT topic
	```c
    pubmsg.payload = str_mqtt;
    pubmsg.payloadlen = strlen(str_mqtt);
    pubmsg.qos = QOS;
    pubmsg.retained = 0;
    MQTTClient_publishMessage(client, TOPIC, &pubmsg, &token);
    rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);
    printf("Message with delivery token %d delivered\n", token);
    ```

8. Here is a link to the [final code](https://github.com/SSG-DRD-IOT/lab-protocols-mqtt-c/edit/master/temperature_mqtt.c).

## Build and Run your C program

Open a SSH terminal to your Intel® IoT Gateway and go to your **/home/nuc-user/labs/protocols/mqtt** folder. Type the following command to build your C application

`gcc temperature_mqtt.c -o temperature_mqtt -I/usr/include/upm -lupmc-temperature -lupmc-utilities -lmraa -lm -lupm-jhd1313m1 -lupmc-jhd1313m1 -lupm-lcm1602 -lupmc-lcm1602 -lpaho-mqtt3c -ljson-c`

To run the program give following command:

`./temperature_mqtt`

This should execute your program and you should see messages like below on your SSH terminal
```c
26 degrees Celsius, or 79 degrees Fahrenheit
Message with delivery token 1 delivered
26 degrees Celsius, or 79 degrees Fahrenheit
Message with delivery token 2 delivered
26 degrees Celsius, or 79 degrees Fahrenheit
Message with delivery token 3 delivered
26 degrees Celsius, or 79 degrees Fahrenheit
Message with delivery token 4 delivered
26 degrees Celsius, or 79 degrees Fahrenheit
Message with delivery token 5 delivered
```

To verify that we are successfully publishing the temperature data over MQTT topic **sensors/temperature/data** open another SSH terminal to your Intel® IoT Gateway. Make sure this program is still running and publishing sensor data

In the new terminal use the below command to subscribe to MQTT topic

`mosquitto_sub -t "sensors/temperature/data" -p 1883`

If the topic was published correctly you should start seeing the payload message as below:

```c
Temperature in Fahrenheit: 79 C & in Celsius: 26
Temperature in Fahrenheit: 79 C & in Celsius: 26
Temperature in Fahrenheit: 79 C & in Celsius: 26
Temperature in Fahrenheit: 79 C & in Celsius: 26
```
You've completed the service that will publish temperature data to the gateway. See the Debugging MQTT section in the sidebar under Additional Information for information on how to verify that MQTT traffic is indeed being published.

# Additional Resources

* [MQTT](http://mqtt.org/)
* [ExpressJS](https://www.npmjs.com/package/express)
* [REST](https://en.wikipedia.org/wiki/Representational_state_transfer)
