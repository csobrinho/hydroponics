#ifndef HYDROPONICS_NETWORK_SYSLOG_H
#define HYDROPONICS_NETWORK_SYSLOG_H

#include <sys/cdefs.h>

#include "esp_err.h"

#include "context.h"

/*!<
 * Priorities/facilities are encoded into a single 32-bit quantity, where the
 * bottom 3 bits are the priority (0-7) and the top 28 bits are the facility
 * (0-big number).  Both the priorities and the facilities map roughly
 * one-to-one to strings in the syslogd(8) source code.  This mapping is
 * included in this file.
 */
typedef enum {
    SYSLOG_PRIORITY_EMERG = 0,   /*!< system is unusable */
    SYSLOG_PRIORITY_ALERT = 1,   /*!< action must be taken immediately */
    SYSLOG_PRIORITY_CRIT = 2,    /*!< critical conditions */
    SYSLOG_PRIORITY_ERR = 3,     /*!< error conditions */
    SYSLOG_PRIORITY_WARNING = 4, /*!< warning conditions */
    SYSLOG_PRIORITY_NOTICE = 5,  /*!< normal but significant condition */
    SYSLOG_PRIORITY_INFO = 6,    /*!< informational */
    SYSLOG_PRIORITY_DEBUG = 7,   /*!< debug-level messages */
} syslog_priority_t;

/*!< Log facility codes. */
typedef enum {
    SYSLOG_FACILITY_KERN = (0 << 3),      /*!< kernel messages */
    SYSLOG_FACILITY_USER = (1 << 3),      /*!< random user-level messages */
    SYSLOG_FACILITY_MAIL = (2 << 3),      /*!< mail system */
    SYSLOG_FACILITY_DAEMON = (3 << 3),    /*!< system daemons */
    SYSLOG_FACILITY_AUTH = (4 << 3),      /*!< security/authorization messages */
    SYSLOG_FACILITY_SYSLOG = (5 << 3),    /*!< messages generated internally by syslogd */
    SYSLOG_FACILITY_LPR = (6 << 3),       /*!< line printer subsystem */
    SYSLOG_FACILITY_NEWS = (7 << 3),      /*!< network news subsystem */
    SYSLOG_FACILITY_UUCP = (8 << 3),      /*!< UUCP subsystem */
    SYSLOG_FACILITY_CRON = (9 << 3),      /*!< clock daemon */
    SYSLOG_FACILITY_AUTHPRIV = (10 << 3), /*!< security/authorization messages (private) */
    SYSLOG_FACILITY_FTP = (11 << 3),      /*!< ftp daemon */
    // other codes through 15 reserved for system use
    SYSLOG_FACILITY_LOCAL0 = (16 << 3),   /*!< reserved for local use */
    SYSLOG_FACILITY_LOCAL1 = (17 << 3),   /*!< reserved for local use */
    SYSLOG_FACILITY_LOCAL2 = (18 << 3),   /*!< reserved for local use */
    SYSLOG_FACILITY_LOCAL3 = (19 << 3),   /*!< reserved for local use */
    SYSLOG_FACILITY_LOCAL4 = (20 << 3),   /*!< reserved for local use */
    SYSLOG_FACILITY_LOCAL5 = (21 << 3),   /*!< reserved for local use */
    SYSLOG_FACILITY_LOCAL6 = (22 << 3),   /*!< reserved for local use */
    SYSLOG_FACILITY_LOCAL7 = (23 << 3),   /*!< reserved for local use */
} syslog_facility_t;

typedef struct {
    const time_t timestamp;
    const syslog_priority_t priority;
    const char *msg;
    const size_t msg_len;
} syslog_entry_t;

esp_err_t syslog_init(context_t *context);

int syslog_vlogf(uint16_t pri, const char *fmt, va_list args) __printflike(2, 0);

#endif //HYDROPONICS_NETWORK_SYSLOG_H
