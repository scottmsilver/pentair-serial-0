// This is a generated file! Please edit source .ksy file and use kaitai-struct-compiler to rebuild

#include "pentair.h"



pentair_t::pentair_t(kaitai::kstream *p_io, kaitai::kstruct* p_parent, pentair_t *p_root) : kaitai::kstruct(p_io) {
  m__parent = p_parent;
  m__root = this;
  _read();
}

void pentair_t::_read() {
  m_messages = new std::vector<message_t*>();
  while (!m__io->is_eof()) {
    m_messages->push_back(new message_t(m__io, this, m__root));
  }
}
pentair_t::~pentair_t() {
  for (std::vector<message_t*>::iterator it = m_messages->begin(); it != m_messages->end(); ++it) {
    delete *it;
  }
  delete m_messages;
}

pentair_t::pump_data_t::pump_data_t(kaitai::kstream *p_io, pentair_t::command_t* p_parent, pentair_t *p_root) : kaitai::kstruct(p_io) {
  m__parent = p_parent;
  m__root = p_root;
  _read();
}

void pentair_t::pump_data_t::_read() {
  m_pump_data_type = m__io->read_u1();
  switch (pump_data_type()) {
    case 2:
      m__raw_body = m__io->read_bytes((_parent()->size() - 1));
      m__io__raw_body = new kaitai::kstream(m__raw_body);
      m_body = new pump_data_rpm_t(m__io__raw_body, this, m__root);
      break;
    case 7:
      m__raw_body = m__io->read_bytes((_parent()->size() - 1));
      m__io__raw_body = new kaitai::kstream(m__raw_body);
      m_body = new pump_ack_t(m__io__raw_body, this, m__root);
      break;
    default:
      m__raw_body = m__io->read_bytes((_parent()->size() - 1));
      break;
  }
}
pentair_t::pump_data_t::~pump_data_t() {
}

pentair_t::heat_set_points_t::heat_set_points_t(kaitai::kstream *p_io, pentair_t::command_t* p_parent, pentair_t *p_root) : kaitai::kstruct(p_io) {
  m__parent = p_parent;
  m__root = p_root;
  _read();
}

void pentair_t::heat_set_points_t::_read() {
  m_pool_temperature = m__io->read_u1();
  m_spa_temperature = m__io->read_u1();
  m_air_temperature = m__io->read_u1();
  m_pool_setpoint = m__io->read_u1();
  m_spa_setpoint = m__io->read_u1();
  m_spa_heat_mode = m__io->read_bits_int(2);
  m_pool_heat_mode = m__io->read_bits_int(2);
  m__io->align_to_byte();
  m_unknown0 = m__io->read_bytes(2);
  m_solar_temperature = m__io->read_u1();
  m_unknown1 = m__io->read_bytes(4);
}
pentair_t::heat_set_points_t::~heat_set_points_t() {
}

pentair_t::pump_data_rpm_t::pump_data_rpm_t(kaitai::kstream *p_io, pentair_t::pump_data_t* p_parent, pentair_t *p_root) : kaitai::kstruct(p_io) {
  m__parent = p_parent;
  m__root = p_root;
  _read();
}

void pentair_t::pump_data_rpm_t::_read() {
  m_rpm = m__io->read_u2be();
}
pentair_t::pump_data_rpm_t::~pump_data_rpm_t() {
}

pentair_t::chlorinator_status_t::chlorinator_status_t(kaitai::kstream *p_io, pentair_t::command_t* p_parent, pentair_t *p_root) : kaitai::kstruct(p_io) {
  m__parent = p_parent;
  m__root = p_root;
  _read();
}

void pentair_t::chlorinator_status_t::_read() {
  m_output_spa_percent = m__io->read_u1();
  m_output_percent = m__io->read_u1();
  m_unknown0 = m__io->read_u1();
  m_salt_ppm = m__io->read_u1();
  m_status = m__io->read_u1();
  m_unknown1 = m__io->read_u1();
  m_name = kaitai::kstream::bytes_to_str(m__io->read_bytes(16), std::string("ASCII"));
}
pentair_t::chlorinator_status_t::~chlorinator_status_t() {
}

pentair_t::pump_power_t::pump_power_t(kaitai::kstream *p_io, pentair_t::command_t* p_parent, pentair_t *p_root) : kaitai::kstruct(p_io) {
  m__parent = p_parent;
  m__root = p_root;
  _read();
}

void pentair_t::pump_power_t::_read() {
  m_status = static_cast<pentair_t::pump_power_t::on_or_off_t>(m__io->read_u1());
}
pentair_t::pump_power_t::~pump_power_t() {
}

pentair_t::chlorinator_unknown0_t::chlorinator_unknown0_t(kaitai::kstream *p_io, pentair_t::command_t* p_parent, pentair_t *p_root) : kaitai::kstruct(p_io) {
  m__parent = p_parent;
  m__root = p_root;
  _read();
}

void pentair_t::chlorinator_unknown0_t::_read() {
  m_unknown = m__io->read_u1();
}
pentair_t::chlorinator_unknown0_t::~chlorinator_unknown0_t() {
}

pentair_t::command_t::command_t(kaitai::kstream *p_io, pentair_t::message_t* p_parent, pentair_t *p_root) : kaitai::kstruct(p_io) {
  m__parent = p_parent;
  m__root = p_root;
  _read();
}

void pentair_t::command_t::_read() {
  m_command_type = m__io->read_u1();
  m_size = m__io->read_u1();
  switch (command_type()) {
    case 4:
      m__raw_body = m__io->read_bytes(size());
      m__io__raw_body = new kaitai::kstream(m__raw_body);
      m_body = new pump_remote_control_t(m__io__raw_body, this, m__root);
      break;
    case 6:
      m__raw_body = m__io->read_bytes(size());
      m__io__raw_body = new kaitai::kstream(m__raw_body);
      m_body = new pump_power_t(m__io__raw_body, this, m__root);
      break;
    case 7:
      m__raw_body = m__io->read_bytes(size());
      m__io__raw_body = new kaitai::kstream(m__raw_body);
      m_body = new pump_status_request_or_response_t(m__io__raw_body, this, m__root);
      break;
    case 1:
      m__raw_body = m__io->read_bytes(size());
      m__io__raw_body = new kaitai::kstream(m__raw_body);
      m_body = new pump_data_t(m__io__raw_body, this, m__root);
      break;
    case 5:
      m__raw_body = m__io->read_bytes(size());
      m__io__raw_body = new kaitai::kstream(m__raw_body);
      m_body = new controller_date_t(m__io__raw_body, this, m__root);
      break;
    case 8:
      m__raw_body = m__io->read_bytes(size());
      m__io__raw_body = new kaitai::kstream(m__raw_body);
      m_body = new heat_set_points_t(m__io__raw_body, this, m__root);
      break;
    case 217:
      m__raw_body = m__io->read_bytes(size());
      m__io__raw_body = new kaitai::kstream(m__raw_body);
      m_body = new chlorinator_unknown0_t(m__io__raw_body, this, m__root);
      break;
    case 2:
      m__raw_body = m__io->read_bytes(size());
      m__io__raw_body = new kaitai::kstream(m__raw_body);
      m_body = new controller_status_t(m__io__raw_body, this, m__root);
      break;
    case 25:
      m__raw_body = m__io->read_bytes(size());
      m__io__raw_body = new kaitai::kstream(m__raw_body);
      m_body = new chlorinator_status_t(m__io__raw_body, this, m__root);
      break;
    default:
      m__raw_body = m__io->read_bytes(size());
      break;
  }
}
pentair_t::command_t::~command_t() {
}

pentair_t::pump_ack_t::pump_ack_t(kaitai::kstream *p_io, pentair_t::pump_data_t* p_parent, pentair_t *p_root) : kaitai::kstruct(p_io) {
  m__parent = p_parent;
  m__root = p_root;
  _read();
}

void pentair_t::pump_ack_t::_read() {
  m_ack = m__io->read_u1();
}
pentair_t::pump_ack_t::~pump_ack_t() {
}

pentair_t::controller_status_t::controller_status_t(kaitai::kstream *p_io, pentair_t::command_t* p_parent, pentair_t *p_root) : kaitai::kstruct(p_io) {
  m__parent = p_parent;
  m__root = p_root;
  _read();
}

void pentair_t::controller_status_t::_read() {
  m_current_time = new time_t(m__io, this, m__root);
  m_feature3_status = m__io->read_bits_int(1);
  m_feature2_status = m__io->read_bits_int(1);
  m_feature1_status = m__io->read_bits_int(1);
  m_pool_status = m__io->read_bits_int(1);
  m_aux3_status = m__io->read_bits_int(1);
  m_aux2_status = m__io->read_bits_int(1);
  m_aux1_status = m__io->read_bits_int(1);
  m_spa_status = m__io->read_bits_int(1);
  m_unknown_equipment = m__io->read_bits_int(7);
  m_aux4_status = m__io->read_bits_int(1);
  m__io->align_to_byte();
  m_equipment_bank_3 = m__io->read_u1();
  m_unknown0 = m__io->read_bytes(4);
  m_status_timeout = m__io->read_bits_int(1);
  m_status_unknown0 = m__io->read_bits_int(3);
  m_status_freeze_protection = m__io->read_bits_int(1);
  m_temperature_units = static_cast<pentair_t::controller_status_t::temperature_units_t>(m__io->read_bits_int(1));
  m_status_unknown1 = m__io->read_bits_int(1);
  m_status_run_mode = static_cast<pentair_t::controller_status_t::run_mode_t>(m__io->read_bits_int(1));
  m__io->align_to_byte();
  m_valves = m__io->read_u1();
  m_unknown1 = m__io->read_bytes(2);
  m_unknown2 = m__io->read_u1();
  m_pool_temp = m__io->read_u1();
  m_spa_temp = m__io->read_u1();
  m_unknown_heater_preference = m__io->read_bits_int(4);
  m_pool_heater_preference = static_cast<pentair_t::controller_status_t::heater_preference_t>(m__io->read_bits_int(2));
  m_spa_heater_preference = static_cast<pentair_t::controller_status_t::heater_preference_t>(m__io->read_bits_int(2));
  m__io->align_to_byte();
  m_unknown3 = m__io->read_bytes(1);
  m_air_temperature = m__io->read_u1();
  m_solar_temperature = m__io->read_u1();
  m_unknown4 = m__io->read_bytes(2);
  m_heater_mode = m__io->read_u1();
  m_unknown5 = m__io->read_bytes(3);
  m_auto_adjust_dst = m__io->read_u1();
  m_unknown6 = m__io->read_bytes(2);
}
pentair_t::controller_status_t::~controller_status_t() {
  delete m_current_time;
}

pentair_t::checksum_t::checksum_t(kaitai::kstream *p_io, pentair_t::message_t* p_parent, pentair_t *p_root) : kaitai::kstruct(p_io) {
  m__parent = p_parent;
  m__root = p_root;
  _read();
}

void pentair_t::checksum_t::_read() {
  m_value = m__io->read_u2be();
}
pentair_t::checksum_t::~checksum_t() {
}

pentair_t::pump_status_response_t::pump_status_response_t(kaitai::kstream *p_io, pentair_t::pump_status_request_or_response_t* p_parent, pentair_t *p_root) : kaitai::kstruct(p_io) {
  m__parent = p_parent;
  m__root = p_root;
  _read();
}

void pentair_t::pump_status_response_t::_read() {
  m_run = m__io->read_u1();
  m_mode = m__io->read_u1();
  m_drive_state = m__io->read_u1();
  m_power_in_watts = m__io->read_u2be();
  m_rpm = m__io->read_u2be();
  m_gpm = m__io->read_u1();
  m_ppc = m__io->read_u1();
  m_unknown0 = m__io->read_u1();
  m_error = m__io->read_u1();
  m_unknown1 = m__io->read_u1();
  m_unknown2 = m__io->read_u1();
  m_time = new time_t(m__io, this, m__root);
}
pentair_t::pump_status_response_t::~pump_status_response_t() {
  delete m_time;
}

pentair_t::address_t::address_t(kaitai::kstream *p_io, pentair_t::message_t* p_parent, pentair_t *p_root) : kaitai::kstruct(p_io) {
  m__parent = p_parent;
  m__root = p_root;
  _read();
}

void pentair_t::address_t::_read() {
  m_destination = m__io->read_u1();
  m_source = m__io->read_u1();
}
pentair_t::address_t::~address_t() {
}

pentair_t::header_t::header_t(kaitai::kstream *p_io, pentair_t::message_t* p_parent, pentair_t *p_root) : kaitai::kstruct(p_io) {
  m__parent = p_parent;
  m__root = p_root;
  _read();
}

void pentair_t::header_t::_read() {
  m_magic = new std::vector<uint8_t>();
  {
    uint8_t _;
    do {
      _ = m__io->read_u1();
      m_magic->push_back(_);
    } while (!(_ == 165));
  }
  m_unknown0 = m__io->read_u1();
}
pentair_t::header_t::~header_t() {
  delete m_magic;
}

pentair_t::message_t::message_t(kaitai::kstream *p_io, pentair_t* p_parent, pentair_t *p_root) : kaitai::kstruct(p_io) {
  m__parent = p_parent;
  m__root = p_root;
  _read();
}

void pentair_t::message_t::_read() {
  m_header = new header_t(m__io, this, m__root);
  m_address = new address_t(m__io, this, m__root);
  m_command = new command_t(m__io, this, m__root);
  m_checksum = new checksum_t(m__io, this, m__root);
}
pentair_t::message_t::~message_t() {
  delete m_header;
  delete m_address;
  delete m_command;
  delete m_checksum;
}

pentair_t::controller_date_t::controller_date_t(kaitai::kstream *p_io, pentair_t::command_t* p_parent, pentair_t *p_root) : kaitai::kstruct(p_io) {
  m__parent = p_parent;
  m__root = p_root;
  _read();
}

void pentair_t::controller_date_t::_read() {
  m_hour = m__io->read_u1();
  m_minute = m__io->read_u1();
  m_day_of_week = static_cast<pentair_t::controller_date_t::day_of_week_t>(m__io->read_u1());
  m_date = m__io->read_u1();
  m_month = m__io->read_u1();
  m_year = m__io->read_u1();
  m_unknown = m__io->read_u1();
  m_auto_adjust_dst = m__io->read_u1();
}
pentair_t::controller_date_t::~controller_date_t() {
}

pentair_t::time_t::time_t(kaitai::kstream *p_io, kaitai::kstruct* p_parent, pentair_t *p_root) : kaitai::kstruct(p_io) {
  m__parent = p_parent;
  m__root = p_root;
  _read();
}

void pentair_t::time_t::_read() {
  m_hour = m__io->read_u1();
  m_minute = m__io->read_u1();
}
pentair_t::time_t::~time_t() {
}

pentair_t::pump_remote_control_t::pump_remote_control_t(kaitai::kstream *p_io, pentair_t::command_t* p_parent, pentair_t *p_root) : kaitai::kstruct(p_io) {
  m__parent = p_parent;
  m__root = p_root;
  _read();
}

void pentair_t::pump_remote_control_t::_read() {
  m_status = static_cast<pentair_t::pump_remote_control_t::on_or_off_t>(m__io->read_u1());
}
pentair_t::pump_remote_control_t::~pump_remote_control_t() {
}

pentair_t::pump_status_request_or_response_t::pump_status_request_or_response_t(kaitai::kstream *p_io, pentair_t::command_t* p_parent, pentair_t *p_root) : kaitai::kstruct(p_io) {
  m__parent = p_parent;
  m__root = p_root;
  _read();
}

void pentair_t::pump_status_request_or_response_t::_read() {
  n_pump_status_response = true;
  if (_parent()->size() > 0) {
    n_pump_status_response = false;
    m_pump_status_response = new pump_status_response_t(m__io, this, m__root);
  }
}
pentair_t::pump_status_request_or_response_t::~pump_status_request_or_response_t() {
  if (!n_pump_status_response) {
    delete m_pump_status_response;
  }
}
