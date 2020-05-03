#include <WiFi.h>
#include <HTTPClient.h>
#include <EEPROM.h>

#define BUF_SIZE 128
#define EEPROM_SIZE 512

char inbuf[BUF_SIZE];
byte inbuf_idx = 0;

// wifi connection
char ssid[BUF_SIZE];
char pass[BUF_SIZE];

// TCP host connection
WiFiClient tcp_client;
char tcp_host[BUF_SIZE];
unsigned short int tcp_port = 0;

void clear_buf(char *buf)
{
  for (int i = 0; i < BUF_SIZE; i++)
  {
    buf[i] = 0;
  }
}

void save_config()
{
  for (int i = 0; i < BUF_SIZE; i++)
  {
    EEPROM.write(i, *(ssid + i));
    EEPROM.write(BUF_SIZE + i, *(pass + i));
    EEPROM.write(BUF_SIZE * 2 + i, *(tcp_host + i));
  }

  EEPROM.write(BUF_SIZE * 3, (byte)tcp_port);
  EEPROM.write(BUF_SIZE * 3 + 1, (byte)(tcp_port >> 8));

  EEPROM.commit();
}

void load_config()
{
  for (int i = 0; i < BUF_SIZE; i++)
  {
    *(ssid + i) = EEPROM.read(i);
    *(pass + i) = EEPROM.read(BUF_SIZE + i);
    *(tcp_host + i) = EEPROM.read(BUF_SIZE * 2 + i);
  }

  tcp_port = EEPROM.read(BUF_SIZE * 3);
  tcp_port += EEPROM.read(BUF_SIZE * 3 + 1) << 8;
}

void parse_cmd(char *cmd)
{
  if (strlen(cmd) == 0)
  {
    return;
  }
  
  if (!strcmp(cmd, "EM$test"))
  {
    // simple test to see if modem is connected
    Serial.println("OK");
  }
  else if (!strncmp(cmd, "EM$ssid", 7))
  {
    // user input of SSID
    strncpy(ssid, cmd + 7, BUF_SIZE - 7);

    Serial.println("OK");
  }
  else if (!strncmp(cmd, "EM$pass", 7))
  {
    // user input of wifi password
    strncpy(pass, cmd + 7, BUF_SIZE - 7);

    Serial.println("OK");
  }
  else if (!strcmp(cmd, "EM$connect"))
  {
    WiFi.begin(ssid, pass);

    Serial.println("Connecting...");
    while (WiFi.status() != WL_CONNECTED);
    Serial.println("OK");
  }
  else if (!strcmp(cmd, "EM$info"))
  {
    Serial.println("EMODEM INFO");
    Serial.println();

    Serial.println("WIFI");
    Serial.print("SSID: ");
    Serial.println(ssid);

    Serial.print("PASS: ");
    Serial.println(pass);

    if (WiFi.status() != WL_CONNECTED)
    {
      Serial.println("NOT CONNECTED");
    }
    else
    {
      Serial.println("CONNECTED");
    }

    Serial.println();
    Serial.println("TCP HOST");
    Serial.print("HOST: ");
    Serial.println(tcp_host);
    Serial.print("PORT: ");
    Serial.println(tcp_port);
    if (tcp_client.connected())
    {
      Serial.println("CONNECTED");
    }
    else
    {
      Serial.println("NOT CONNECTED");
    }

    Serial.println();
    Serial.println("OK");
  }
  else if (!strcmp(cmd, "EM$address"))
  {
    // display user's IP addresses

    if (WiFi.status() != WL_CONNECTED)
    {
      Serial.println("NOT CONNECTED");
      return;
    }

    // print local IP
    Serial.println(WiFi.localIP());

    // print public IP
    HTTPClient http;
    http.begin("http://api.ipify.org/");

    if (http.GET() > 0)
    {
      Serial.println(http.getString());
      Serial.println("OK");
    }
    else
    {
      Serial.println("ERROR");
    }
  }
  else if (!strncmp(cmd, "EM$host", 7))
  {
    // user specified TCP host
    strncpy(tcp_host, cmd + 7, BUF_SIZE - 7);

    Serial.println("OK");
  }
  else if (!strncmp(cmd, "EM$port", 7))
  {
    // user specified TCP port
    tcp_port = atoi(cmd + 7);

    Serial.println("OK");
  }
  else if (!strcmp(cmd, "EM$tcpconnect"))
  {
    if (WiFi.status() != WL_CONNECTED)
    {
      Serial.println("NOT CONNECTED TO WIFI");
      return;
    }

    // attempt TCP connection
    Serial.print("Connecting to ");
    Serial.println(tcp_host);

    if (!tcp_client.connect(tcp_host, tcp_port))
    {
      Serial.println("ERROR");
      return;
    }

    Serial.println("OK");
  }
  else if (!strcmp(cmd, "EM$save"))
  {
    // save user config
    save_config();

    Serial.println("OK");
  }
  else if (!strcmp(cmd, "EM$load"))
  {
    // load user config
    load_config();

    Serial.println("OK");
  }
  else
  {
    Serial.println("COMMAND NOT RECOGNIZED");
  }
}

void setup() {
  Serial.begin(2400);
  clear_buf(inbuf);
  clear_buf(ssid);
  clear_buf(pass);
  clear_buf(tcp_host);
  tcp_port = 80;

  if (!EEPROM.begin(EEPROM_SIZE))
  {
    Serial.println("EEPROM FAIL");
    for(;;);
  }

  // automatically load any saved config
  load_config();

  Serial.println("EMODEM READY");
}

void loop() {
  if (tcp_client.connected())
  {
    // direct I/O to the TCP connection

    if (tcp_client.available() > 0)
    {
      Serial.write(tcp_client.read());
    }

    if (Serial.available() > 0)
    {
      tcp_client.write(Serial.read());
    }
  }
  else
  {
    // direct I/O to input buffer

    if (Serial.available() > 0)
    {
      byte inb = Serial.read();

      Serial.write(inb);
      if (inb == 0x0D || inbuf_idx == BUF_SIZE - 1)
      {
        // carriage return
        inbuf[inbuf_idx] = 0;
        parse_cmd(inbuf);

        clear_buf(inbuf);
        inbuf_idx = 0;
      }
      else if (inb == 0x08)
      {
        // backspace
        if (inbuf_idx > 0)
          inbuf_idx--;
          inbuf[inbuf_idx] = 0;
      }
      else
      {
        inbuf[inbuf_idx] = inb;
        inbuf_idx++;
      }
    }
  }
}
