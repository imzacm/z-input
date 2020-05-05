#include <stdio.h>
#include <3ds.h>
#include <string.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <malloc.h>

#define SOC_ALIGN       0x1000
#define SOC_BUFFERSIZE  0x100000

int connect(char* address, int port) {
    struct sockaddr_in server_addr = { 0 };
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    int ret = inet_pton(AF_INET, address, &server_addr.sin_addr);
    if (ret != 1) {
        printf("Error: Failed to convert address\n");
		return -1;
    }
    fprintf(stdout, "CONNECTING: address=%s port=%d\n", address, port);
    int conn_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (conn_fd == -1) {
        printf("Error:F Failed to create to socket\n");
	    return -1;
    }
    ret = connect(conn_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (ret == -1) {
        printf("Error: Failed to initialise socket\n");
	    return -1;
    }
	printf("Connected\n");
    return conn_fd;
}

int main() {
    gfxInitDefault();
    consoleInit(GFX_TOP, NULL);
    hidInit();
	
	static u32 *SOC_buffer = NULL;
    SOC_buffer = (u32*)memalign(SOC_ALIGN, SOC_BUFFERSIZE);
    if(SOC_buffer == NULL) {
        printf("Error: Failed to initialise SOC buffer memory\n");
    }
    if (socInit(SOC_buffer, SOC_BUFFERSIZE) != 0) {
        printf("Error: Failed to initialise SOC buffer\n");
    }

	bool socket_connected = false;
	int conn_fd;
    
    char address_buffer[32] = {'\0'};
    char port_buffer[5] = {'\0'};
	int loops_since_last_send = -1;

    while (aptMainLoop()) {
		loops_since_last_send++;
		// printf("%d loops since last send\n", loops_since_last_send);
		
		if (!socket_connected) {
			SwkbdState address_kbd_state;
            swkbdInit(&address_kbd_state, SWKBD_TYPE_QWERTY, 3, 32);
			swkbdSetHintText(&address_kbd_state, "IP address");
			if (strlen(address_buffer) > 0) {
				swkbdSetInitialText(&address_kbd_state, address_buffer);
			}
            swkbdInputText(&address_kbd_state, address_buffer, sizeof(address_buffer));
            swkbdGetResult(&address_kbd_state);
			
			SwkbdState port_kbd_state;
            swkbdInit(&port_kbd_state, SWKBD_TYPE_NUMPAD, 3, 5);
			swkbdSetHintText(&port_kbd_state, "Port");
			if (strlen(port_buffer) > 0) {
				swkbdSetInitialText(&port_kbd_state, port_buffer);
			}
            swkbdInputText(&port_kbd_state, port_buffer, sizeof(port_buffer));
            swkbdGetResult(&port_kbd_state);
			int port = atoi(port_buffer);
			
			conn_fd = connect(address_buffer, port);
			socket_connected = conn_fd != -1;
			if (!socket_connected) {
				gfxFlushBuffers();
				gfxSwapBuffers();
				gspWaitForVBlank();
				continue;
			}
		}
		
        hidScanInput();
        u32 kDown = hidKeysDown();
		u32 kHeld = hidKeysHeld();
		
		circlePosition circle_point = {0, 0};
		hidCircleRead(&circle_point);
		
		auto is_pressed = [&](auto key) -> bool {
			return (kDown & key) || (kHeld & key && loops_since_last_send > 15);
		};
        
        touchPosition touch = {0, 0};
        hidTouchRead(&touch);
        
        char buffer[256] = {'\0'};
		
		if (circle_point.dx != 0 || circle_point.dy != 0) {
			
		}
        if (touch.px != 0 || touch.py != 0) {
            SwkbdState kbd_state;
            swkbdInit(&kbd_state, SWKBD_TYPE_QWERTY, 3, -1);
            swkbdInputText(&kbd_state, buffer, sizeof(buffer));
            swkbdGetResult(&kbd_state);
        }
        else if (is_pressed(KEY_A)) {
            buffer[0] = 'A';
        }
        else if (is_pressed(KEY_B)) {
            buffer[0] = 'B';
        }
        else if (is_pressed(KEY_DRIGHT)) {
            buffer[0] = '#';
            buffer[1] = 'R';
        }
        else if (is_pressed(KEY_DLEFT)) {
            buffer[0] = '#';
            buffer[1] = 'L';
        }
        else if (is_pressed(KEY_DUP)) {
            buffer[0] = '#';
            buffer[1] = 'U';
        }
        else if (is_pressed(KEY_DDOWN)) {
            buffer[0] = '#';
            buffer[1] = 'D';
        }
        else if (is_pressed(KEY_R)) {
            buffer[0] = 'R';
        }
        else if (is_pressed(KEY_L)) {
            buffer[0] = 'L';
        }
        else if (is_pressed(KEY_X)) {
            buffer[0] = 'X';
        }
        else if (is_pressed(KEY_Y)) {
            buffer[0] = 'Y';
        }
        else if (is_pressed(KEY_SELECT)) {
            buffer[0] = '>';
        }
        else if (is_pressed(KEY_START)) {
            buffer[0] = '<';
        }
		
		buffer[strlen(buffer)] = '^';
        
        int len;
        if ((len = strlen(buffer)) > 1) {
            printf("Sending: %s\n", buffer);
            int n = write(conn_fd, buffer, len);
            if (n < 0) {
                printf("Failed to send\n");
				shutdown(conn_fd, SHUT_RDWR);
                close(conn_fd);
				socket_connected = false;
            }
            else {
                printf("Sent\n");
            }
			loops_since_last_send = -1;
        }
        

        gfxFlushBuffers();
        gfxSwapBuffers();

        gspWaitForVBlank();
    }

    if (socket_connected) {
        shutdown(conn_fd, SHUT_RDWR);
        close(conn_fd);
    }

    hidExit();
    gfxExit();
    socExit();

    return 0;
}
