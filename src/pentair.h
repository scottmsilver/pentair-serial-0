#ifndef PENTAIR_H_
#define PENTAIR_H_

// This is a generated file! Please edit source .ksy file and use kaitai-struct-compiler to rebuild

#include <kaitai/kaitaistruct.h>
#include <kaitai/kaitaistream.h>

#include <stdint.h>
#include <vector>

#if KAITAI_STRUCT_VERSION < 7000L
#error "Incompatible Kaitai Struct C++/STL API: version 0.7 or later is required"
#endif

class pentair_t : public kaitai::kstruct {

public:
    class pump_data_t;
    class heat_set_points_t;
    class pump_data_rpm_t;
    class chlorinator_status_t;
    class pump_power_t;
    class chlorinator_unknown0_t;
    class command_t;
    class pump_ack_t;
    class controller_status_t;
    class checksum_t;
    class pump_status_response_t;
    class address_t;
    class header_t;
    class message_t;
    class controller_date_t;
    class time_t;
    class pump_remote_control_t;
    class pump_status_request_or_response_t;

    pentair_t(kaitai::kstream* p_io, kaitai::kstruct* p_parent = 0, pentair_t* p_root = 0);
    void _read();
    ~pentair_t();

    class pump_data_t : public kaitai::kstruct {

    public:

        pump_data_t(kaitai::kstream* p_io, pentair_t::command_t* p_parent = 0, pentair_t* p_root = 0);
        void _read();
        ~pump_data_t();

    private:
        uint8_t m_pump_data_type;
        kaitai::kstruct* m_body;
        bool n_body;

    public:
        bool _is_null_body() { body(); return n_body; };

    private:
        pentair_t* m__root;
        pentair_t::command_t* m__parent;
        std::string m__raw_body;
        kaitai::kstream* m__io__raw_body;

    public:

        /**
         * Tells us the type of the data structure we're going to see.
         */
        uint8_t pump_data_type() const { return m_pump_data_type; }
        kaitai::kstruct* body() const { return m_body; }
        pentair_t* _root() const { return m__root; }
        pentair_t::command_t* _parent() const { return m__parent; }
        std::string _raw_body() const { return m__raw_body; }
        kaitai::kstream* _io__raw_body() const { return m__io__raw_body; }
    };

    class heat_set_points_t : public kaitai::kstruct {

    public:

        heat_set_points_t(kaitai::kstream* p_io, pentair_t::command_t* p_parent = 0, pentair_t* p_root = 0);
        void _read();
        ~heat_set_points_t();

    private:
        uint8_t m_pool_temperature;
        uint8_t m_spa_temperature;
        uint8_t m_air_temperature;
        uint8_t m_pool_setpoint;
        uint8_t m_spa_setpoint;
        uint64_t m_spa_heat_mode;
        uint64_t m_pool_heat_mode;
        std::string m_unknown0;
        uint8_t m_solar_temperature;
        std::string m_unknown1;
        pentair_t* m__root;
        pentair_t::command_t* m__parent;

    public:

        /**
         * Current pool temperature
         */
        uint8_t pool_temperature() const { return m_pool_temperature; }

        /**
         * Current spa temperature
         */
        uint8_t spa_temperature() const { return m_spa_temperature; }

        /**
         * Current air temperature.
         */
        uint8_t air_temperature() const { return m_air_temperature; }

        /**
         * Turn on the pool heater if the heat falls below this.
         */
        uint8_t pool_setpoint() const { return m_pool_setpoint; }

        /**
         * Turn on the spa heater if the heat falls below this.
         */
        uint8_t spa_setpoint() const { return m_spa_setpoint; }

        /**
         * Set to non-zero if the spa heater is on.
         */
        uint64_t spa_heat_mode() const { return m_spa_heat_mode; }

        /**
         * Set to non-zero if the pool heater is on.
         */
        uint64_t pool_heat_mode() const { return m_pool_heat_mode; }
        std::string unknown0() const { return m_unknown0; }
        uint8_t solar_temperature() const { return m_solar_temperature; }
        std::string unknown1() const { return m_unknown1; }
        pentair_t* _root() const { return m__root; }
        pentair_t::command_t* _parent() const { return m__parent; }
    };

    class pump_data_rpm_t : public kaitai::kstruct {

    public:

        pump_data_rpm_t(kaitai::kstream* p_io, pentair_t::pump_data_t* p_parent = 0, pentair_t* p_root = 0);
        void _read();
        ~pump_data_rpm_t();

    private:
        uint16_t m_rpm;
        pentair_t* m__root;
        pentair_t::pump_data_t* m__parent;

    public:
        uint16_t rpm() const { return m_rpm; }
        pentair_t* _root() const { return m__root; }
        pentair_t::pump_data_t* _parent() const { return m__parent; }
    };

    class chlorinator_status_t : public kaitai::kstruct {

    public:

        chlorinator_status_t(kaitai::kstream* p_io, pentair_t::command_t* p_parent = 0, pentair_t* p_root = 0);
        void _read();
        ~chlorinator_status_t();

    private:
        uint8_t m_output_spa_percent;
        uint8_t m_output_percent;
        uint8_t m_unknown0;
        uint8_t m_salt_ppm;
        uint8_t m_status;
        uint8_t m_unknown1;
        std::string m_name;
        pentair_t* m__root;
        pentair_t::command_t* m__parent;

    public:
        uint8_t output_spa_percent() const { return m_output_spa_percent; }
        uint8_t output_percent() const { return m_output_percent; }
        uint8_t unknown0() const { return m_unknown0; }
        uint8_t salt_ppm() const { return m_salt_ppm; }
        uint8_t status() const { return m_status; }
        uint8_t unknown1() const { return m_unknown1; }
        std::string name() const { return m_name; }
        pentair_t* _root() const { return m__root; }
        pentair_t::command_t* _parent() const { return m__parent; }
    };

    class pump_power_t : public kaitai::kstruct {

    public:

        enum on_or_off_t {
            ON_OR_OFF_FALSE = 4,
            ON_OR_OFF_TRUE = 10
        };

        pump_power_t(kaitai::kstream* p_io, pentair_t::command_t* p_parent = 0, pentair_t* p_root = 0);
        void _read();
        ~pump_power_t();

    private:
        on_or_off_t m_status;
        pentair_t* m__root;
        pentair_t::command_t* m__parent;

    public:
        on_or_off_t status() const { return m_status; }
        pentair_t* _root() const { return m__root; }
        pentair_t::command_t* _parent() const { return m__parent; }
    };

    class chlorinator_unknown0_t : public kaitai::kstruct {

    public:

        chlorinator_unknown0_t(kaitai::kstream* p_io, pentair_t::command_t* p_parent = 0, pentair_t* p_root = 0);
        void _read();
        ~chlorinator_unknown0_t();

    private:
        uint8_t m_status;
        pentair_t* m__root;
        pentair_t::command_t* m__parent;

    public:
        uint8_t status() const { return m_status; }
        pentair_t* _root() const { return m__root; }
        pentair_t::command_t* _parent() const { return m__parent; }
    };

    class command_t : public kaitai::kstruct {

    public:

        enum command_type_t {
            COMMAND_TYPE_PUMP_DATA_TYPE = 1,
            COMMAND_TYPE_CONTROLLER_STATUS_TYPE = 2,
            COMMAND_TYPE_PUMP_REMOTE_CONTROL_TYPE = 4,
            COMMAND_TYPE_CONTROLLER_DATE_TYPE = 5,
            COMMAND_TYPE_PUMP_POWER_TYPE = 6,
            COMMAND_TYPE_PUMP_STATUS_REQUEST_OR_RESPONSE_TYPE = 7,
            COMMAND_TYPE_HEAT_SET_POINTS_TYPE = 8,
            COMMAND_TYPE_CHLORINATOR_STATUS_TYPE = 25,
            COMMAND_TYPE_CHLORINATOR_UNKNOWN0_TYPE = 217
        };

        command_t(kaitai::kstream* p_io, pentair_t::message_t* p_parent = 0, pentair_t* p_root = 0);
        void _read();
        ~command_t();

    private:
        command_type_t m_type;
        uint8_t m_size;
        kaitai::kstruct* m_body;
        bool n_body;

    public:
        bool _is_null_body() { body(); return n_body; };

    private:
        pentair_t* m__root;
        pentair_t::message_t* m__parent;
        std::string m__raw_body;
        kaitai::kstream* m__io__raw_body;

    public:

        /**
         * type of command
         */
        command_type_t type() const { return m_type; }

        /**
         * size of command message
         */
        uint8_t size() const { return m_size; }
        kaitai::kstruct* body() const { return m_body; }
        pentair_t* _root() const { return m__root; }
        pentair_t::message_t* _parent() const { return m__parent; }
        std::string _raw_body() const { return m__raw_body; }
        kaitai::kstream* _io__raw_body() const { return m__io__raw_body; }
    };

    class pump_ack_t : public kaitai::kstruct {

    public:

        pump_ack_t(kaitai::kstream* p_io, pentair_t::pump_data_t* p_parent = 0, pentair_t* p_root = 0);
        void _read();
        ~pump_ack_t();

    private:
        uint8_t m_ack;
        pentair_t* m__root;
        pentair_t::pump_data_t* m__parent;

    public:
        uint8_t ack() const { return m_ack; }
        pentair_t* _root() const { return m__root; }
        pentair_t::pump_data_t* _parent() const { return m__parent; }
    };

    class controller_status_t : public kaitai::kstruct {

    public:

        enum temperature_units_t {
            TEMPERATURE_UNITS_FARENHEIT = 0,
            TEMPERATURE_UNITS_CELCIUS = 4
        };

        enum heater_preference_t {
            HEATER_PREFERENCE_FALSE = 0,
            HEATER_PREFERENCE_HEATER_ONLY = 1,
            HEATER_PREFERENCE_SOLAR_PREFERRED = 2,
            HEATER_PREFERENCE_SOLAR_ONLY = 3
        };

        enum run_mode_t {
            RUN_MODE_AUTO = 0,
            RUN_MODE_SERVICE = 1
        };

        controller_status_t(kaitai::kstream* p_io, pentair_t::command_t* p_parent = 0, pentair_t* p_root = 0);
        void _read();
        ~controller_status_t();

    private:
        time_t* m_current_time;
        bool m_feature3_status;
        bool m_feature2_status;
        bool m_feature1_status;
        bool m_pool_status;
        bool m_aux3_status;
        bool m_aux2_status;
        bool m_aux1_status;
        bool m_spa_status;
        uint64_t m_unknown_equipment;
        bool m_aux4_status;
        uint8_t m_equipment_bank_3;
        std::string m_unknown0;
        bool m_status_timeout;
        uint64_t m_status_unknown0;
        bool m_status_freeze_protection;
        temperature_units_t m_temperature_units;
        bool m_status_unknown1;
        run_mode_t m_status_run_mode;
        uint8_t m_valves;
        std::string m_unknown1;
        uint8_t m_unknown2;
        uint8_t m_pool_temp;
        uint8_t m_spa_temp;
        uint64_t m_unknown_heater_preference;
        heater_preference_t m_pool_heater_preference;
        heater_preference_t m_spa_heater_preference;
        std::string m_unknown3;
        uint8_t m_air_temperature;
        uint8_t m_solar_temperature;
        std::string m_unknown4;
        uint8_t m_heater_mode;
        std::string m_unknown5;
        uint8_t m_auto_adjust_dst;
        std::string m_unknown6;
        pentair_t* m__root;
        pentair_t::command_t* m__parent;

    public:

        /**
         * The current time.
         */
        time_t* current_time() const { return m_current_time; }

        /**
         * Set if Feature3 is on.
         */
        bool feature3_status() const { return m_feature3_status; }

        /**
         * Set is Feature2 is on
         */
        bool feature2_status() const { return m_feature2_status; }

        /**
         * Set if Feature1 is on.
         */
        bool feature1_status() const { return m_feature1_status; }

        /**
         * Set if Pool is on.
         */
        bool pool_status() const { return m_pool_status; }
        bool aux3_status() const { return m_aux3_status; }

        /**
         * Set if AUX2 is on. AUX2 is usually the pool cleaner.
         */
        bool aux2_status() const { return m_aux2_status; }

        /**
         * Set if AUX3 is on. AUX1 is usually the light.
         */
        bool aux1_status() const { return m_aux1_status; }
        bool spa_status() const { return m_spa_status; }

        /**
         * Presumably this is expansion room for more equipment.
         */
        uint64_t unknown_equipment() const { return m_unknown_equipment; }

        /**
         * Set if AUX4 is on.
         */
        bool aux4_status() const { return m_aux4_status; }

        /**
         * equipment 3
         */
        uint8_t equipment_bank_3() const { return m_equipment_bank_3; }
        std::string unknown0() const { return m_unknown0; }
        bool status_timeout() const { return m_status_timeout; }
        uint64_t status_unknown0() const { return m_status_unknown0; }
        bool status_freeze_protection() const { return m_status_freeze_protection; }

        /**
         * The units of the temperature values in the messages.
         */
        temperature_units_t temperature_units() const { return m_temperature_units; }
        bool status_unknown1() const { return m_status_unknown1; }
        run_mode_t status_run_mode() const { return m_status_run_mode; }
        uint8_t valves() const { return m_valves; }
        std::string unknown1() const { return m_unknown1; }

        /**
         * According to tagyoureit@ it has something to do with heat.
         */
        uint8_t unknown2() const { return m_unknown2; }

        /**
         * The temperature of the pool in chosen units.
         */
        uint8_t pool_temp() const { return m_pool_temp; }

        /**
         * The temperature of the spa in chosen units.
         */
        uint8_t spa_temp() const { return m_spa_temp; }
        uint64_t unknown_heater_preference() const { return m_unknown_heater_preference; }
        heater_preference_t pool_heater_preference() const { return m_pool_heater_preference; }
        heater_preference_t spa_heater_preference() const { return m_spa_heater_preference; }
        std::string unknown3() const { return m_unknown3; }

        /**
         * The temperature of air from thermometer in the chosen units.
         */
        uint8_t air_temperature() const { return m_air_temperature; }

        /**
         * The ? temperature
         */
        uint8_t solar_temperature() const { return m_solar_temperature; }
        std::string unknown4() const { return m_unknown4; }
        uint8_t heater_mode() const { return m_heater_mode; }
        std::string unknown5() const { return m_unknown5; }

        /**
         * 1 if we should auto adjust for daylight savings time - pretty sure this is wrong and is 2 more bits over
         */
        uint8_t auto_adjust_dst() const { return m_auto_adjust_dst; }
        std::string unknown6() const { return m_unknown6; }
        pentair_t* _root() const { return m__root; }
        pentair_t::command_t* _parent() const { return m__parent; }
    };

    class checksum_t : public kaitai::kstruct {

    public:

        checksum_t(kaitai::kstream* p_io, pentair_t::message_t* p_parent = 0, pentair_t* p_root = 0);
        void _read();
        ~checksum_t();

    private:
        uint16_t m_value;
        pentair_t* m__root;
        pentair_t::message_t* m__parent;

    public:
        uint16_t value() const { return m_value; }
        pentair_t* _root() const { return m__root; }
        pentair_t::message_t* _parent() const { return m__parent; }
    };

    class pump_status_response_t : public kaitai::kstruct {

    public:

        pump_status_response_t(kaitai::kstream* p_io, pentair_t::pump_status_request_or_response_t* p_parent = 0, pentair_t* p_root = 0);
        void _read();
        ~pump_status_response_t();

    private:
        uint8_t m_run;
        uint8_t m_mode;
        uint8_t m_drive_state;
        uint16_t m_power_in_watts;
        uint16_t m_rpm;
        uint8_t m_gpm;
        uint8_t m_ppc;
        uint8_t m_unknown0;
        uint8_t m_error;
        uint8_t m_unknown1;
        uint8_t m_unknown2;
        time_t* m_time;
        pentair_t* m__root;
        pentair_t::pump_status_request_or_response_t* m__parent;

    public:
        uint8_t run() const { return m_run; }
        uint8_t mode() const { return m_mode; }
        uint8_t drive_state() const { return m_drive_state; }
        uint16_t power_in_watts() const { return m_power_in_watts; }
        uint16_t rpm() const { return m_rpm; }
        uint8_t gpm() const { return m_gpm; }
        uint8_t ppc() const { return m_ppc; }
        uint8_t unknown0() const { return m_unknown0; }
        uint8_t error() const { return m_error; }
        uint8_t unknown1() const { return m_unknown1; }
        uint8_t unknown2() const { return m_unknown2; }
        time_t* time() const { return m_time; }
        pentair_t* _root() const { return m__root; }
        pentair_t::pump_status_request_or_response_t* _parent() const { return m__parent; }
    };

    class address_t : public kaitai::kstruct {

    public:

        address_t(kaitai::kstream* p_io, pentair_t::message_t* p_parent = 0, pentair_t* p_root = 0);
        void _read();
        ~address_t();

    private:
        uint8_t m_destination;
        uint8_t m_source;
        pentair_t* m__root;
        pentair_t::message_t* m__parent;

    public:

        /**
         * destination address
         */
        uint8_t destination() const { return m_destination; }

        /**
         * source address
         */
        uint8_t source() const { return m_source; }
        pentair_t* _root() const { return m__root; }
        pentair_t::message_t* _parent() const { return m__parent; }
    };

    class header_t : public kaitai::kstruct {

    public:

        header_t(kaitai::kstream* p_io, pentair_t::message_t* p_parent = 0, pentair_t* p_root = 0);
        void _read();
        ~header_t();

    private:
        std::vector<uint8_t>* m_magic;
        uint8_t m_unknown0;
        pentair_t* m__root;
        pentair_t::message_t* m__parent;

    public:

        /**
         * We treat a mesage as any bytes ending in 0xa5.
         */
        std::vector<uint8_t>* magic() const { return m_magic; }
        uint8_t unknown0() const { return m_unknown0; }
        pentair_t* _root() const { return m__root; }
        pentair_t::message_t* _parent() const { return m__parent; }
    };

    class message_t : public kaitai::kstruct {

    public:

        message_t(kaitai::kstream* p_io, pentair_t* p_parent = 0, pentair_t* p_root = 0);
        void _read();
        ~message_t();

    private:
        header_t* m_header;
        address_t* m_address;
        command_t* m_command;
        checksum_t* m_checksum;
        pentair_t* m__root;
        pentair_t* m__parent;

    public:
        header_t* header() const { return m_header; }
        address_t* address() const { return m_address; }
        command_t* command() const { return m_command; }
        checksum_t* checksum() const { return m_checksum; }
        pentair_t* _root() const { return m__root; }
        pentair_t* _parent() const { return m__parent; }
    };

    class controller_date_t : public kaitai::kstruct {

    public:

        enum day_of_week_t {
            DAY_OF_WEEK_SUNDAY = 1,
            DAY_OF_WEEK_MONDAY = 2,
            DAY_OF_WEEK_TUESDAY = 4,
            DAY_OF_WEEK_WEDNESDAY = 8,
            DAY_OF_WEEK_THURSDAY = 16,
            DAY_OF_WEEK_FRIDAY = 32,
            DAY_OF_WEEK_SATURDAY = 64
        };

        controller_date_t(kaitai::kstream* p_io, pentair_t::command_t* p_parent = 0, pentair_t* p_root = 0);
        void _read();
        ~controller_date_t();

    private:
        uint8_t m_hour;
        uint8_t m_minute;
        day_of_week_t m_day_of_week;
        uint8_t m_date;
        uint8_t m_month;
        uint8_t m_year;
        uint8_t m_unknown;
        uint8_t m_auto_adjust_dst;
        pentair_t* m__root;
        pentair_t::command_t* m__parent;

    public:

        /**
         * Hours of current time.
         */
        uint8_t hour() const { return m_hour; }

        /**
         * Minutes of current time.
         */
        uint8_t minute() const { return m_minute; }
        day_of_week_t day_of_week() const { return m_day_of_week; }
        uint8_t date() const { return m_date; }
        uint8_t month() const { return m_month; }

        /**
         * 2 digit year - apparently we're angling for a year 2100 problem...
         */
        uint8_t year() const { return m_year; }
        uint8_t unknown() const { return m_unknown; }

        /**
         * Should we auto adjust to Dst.
         */
        uint8_t auto_adjust_dst() const { return m_auto_adjust_dst; }
        pentair_t* _root() const { return m__root; }
        pentair_t::command_t* _parent() const { return m__parent; }
    };

    class time_t : public kaitai::kstruct {

    public:

        time_t(kaitai::kstream* p_io, kaitai::kstruct* p_parent = 0, pentair_t* p_root = 0);
        void _read();
        ~time_t();

    private:
        uint8_t m_hour;
        uint8_t m_minute;
        pentair_t* m__root;
        kaitai::kstruct* m__parent;

    public:
        uint8_t hour() const { return m_hour; }
        uint8_t minute() const { return m_minute; }
        pentair_t* _root() const { return m__root; }
        kaitai::kstruct* _parent() const { return m__parent; }
    };

    class pump_remote_control_t : public kaitai::kstruct {

    public:

        enum on_or_off_t {
            ON_OR_OFF_FALSE = 0,
            ON_OR_OFF_TRUE = 255
        };

        pump_remote_control_t(kaitai::kstream* p_io, pentair_t::command_t* p_parent = 0, pentair_t* p_root = 0);
        void _read();
        ~pump_remote_control_t();

    private:
        on_or_off_t m_status;
        pentair_t* m__root;
        pentair_t::command_t* m__parent;

    public:
        on_or_off_t status() const { return m_status; }
        pentair_t* _root() const { return m__root; }
        pentair_t::command_t* _parent() const { return m__parent; }
    };

    class pump_status_request_or_response_t : public kaitai::kstruct {

    public:

        pump_status_request_or_response_t(kaitai::kstream* p_io, pentair_t::command_t* p_parent = 0, pentair_t* p_root = 0);
        void _read();
        ~pump_status_request_or_response_t();

    private:
        pump_status_response_t* m_pump_status_response;
        bool n_pump_status_response;

    public:
        bool _is_null_pump_status_response() { pump_status_response(); return n_pump_status_response; };

    private:
        pentair_t* m__root;
        pentair_t::command_t* m__parent;

    public:

        /**
         * This is a response iff the message length is > 0, otherwise it's a request.
         */
        pump_status_response_t* pump_status_response() const { return m_pump_status_response; }
        pentair_t* _root() const { return m__root; }
        pentair_t::command_t* _parent() const { return m__parent; }
    };

private:
    std::vector<message_t*>* m_messages;
    pentair_t* m__root;
    kaitai::kstruct* m__parent;

public:
    std::vector<message_t*>* messages() const { return m_messages; }
    pentair_t* _root() const { return m__root; }
    kaitai::kstruct* _parent() const { return m__parent; }
};

#endif  // PENTAIR_H_
