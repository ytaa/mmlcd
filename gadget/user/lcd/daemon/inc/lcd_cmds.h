#ifndef LCD_CMDS_H
#define LCD_CMDS_H

int lcd_cmds_init(void);

void lcd_cmds_deinit(void);

int lcd_cmds_handle_request(int client_fd);

#endif /* LCD_CMDS_H */