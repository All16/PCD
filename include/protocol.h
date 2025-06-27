#ifndef PROTOCOL_H
#define PROTOCOL_H

#define CMD_PREFIX_CUT "cut"
#define CMD_PREFIX_CONVERT "convert"
#define CMD_PREFIX_EXTRACT "extract_audio"

// Define CommandType or include the appropriate header
typedef enum {
	CMD_CUT,
	CMD_CONVERT,
	CMD_EXTRACT_AUDIO,
	CMD_UNKNOWN
} CommandType;

CommandType parse_command(const char* cmd_str);

#endif