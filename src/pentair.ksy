meta:
  id: pentair
  file-extension: pentair.ksy
  endian: be
seq:
  - id: messages
    type: message
    repeat: eos
types:
  message:
    seq:
      - id: header
        type: header
      - id: address
        type: address
      - id: command
        type: command
      - id: checksum
        type: checksum
  header:
    seq:
     - id: magic
       type: u1
       repeat: until
       repeat-until: _ == 0xa5
       doc: We treat a mesage as any bytes ending in 0xa5.
     - id: unknown0
       type: u1
  address:
    seq:
      - id: destination
        type: u1
        doc: destination address
      - id: source
        type: u1
        doc: source address
  checksum:
    seq:
      - id: value
        type: u2
  command:
    seq:
      - id: type
        type: u1
        doc: type of command
        enum: command_type
      - id: size
        type: u1
        doc: size of command message
      - id: body
        size: size
        type:
          switch-on: type
          cases:
            'command_type::pump_data_type': pump_data
            'command_type::controller_status_type': controller_status
            'command_type::pump_remote_control_type': pump_remote_control
            'command_type::controller_date_type': controller_date
            'command_type::pump_power_type': pump_power
            'command_type::pump_status_request_or_response_type': pump_status_request_or_response
            'command_type::heat_set_points_type': heat_set_points
            'command_type::chlorinator_status_type': chlorinator_status
            'command_type::chlorinator_unknown0_type': chlorinator_unknown0
    enums:
      command_type:
        1: pump_data_type
        2: controller_status_type
        4: pump_remote_control_type
        5: controller_date_type
        6: pump_power_type
        7: pump_status_request_or_response_type
        8: heat_set_points_type
        25: chlorinator_status_type
        217: chlorinator_unknown0_type
  controller_status:
    seq:
     - id: current_time
       type: time
       doc: The current time.
     - id: feature3_status
       type: b1
       doc: Set if Feature3 is on.
     - id: feature2_status
       type: b1
       doc: Set is Feature2 is on
     - id: feature1_status
       type: b1
       doc: Set if Feature1 is on.
     - id: pool_status
       type: b1
       doc: Set if Pool is on.
     - id: aux3_status
       type: b1
     - id: aux2_status
       type: b1
       doc: Set if AUX2 is on. AUX2 is usually the pool cleaner.
     - id: aux1_status
       type: b1
       doc: Set if AUX3 is on. AUX1 is usually the light.
     - id: spa_status
       type: b1
     - id: unknown_equipment
       type: b7
       doc: Presumably this is expansion room for more equipment.
     - id: aux4_status
       type: b1
       doc: Set if AUX4 is on.
     - id: equipment_bank_3
       type: u1
       doc: equipment 3
     - id: unknown0
       size: 4
     - id: status_timeout
       type: b1
     - id: status_unknown0
       type: b3
     - id: status_freeze_protection
       type: b1
     - id: temperature_units
       type: b1
       enum: temperature_units
       doc: The units of the temperature values in the messages.
     - id: status_unknown1
       type: b1
     - id: status_run_mode
       type: b1
       enum: run_mode
     - id: valves
       type: u1
     - id: unknown1
       size: 2
     - id: unknown2
       type: u1
       doc: According to tagyoureit@ it has something to do with heat.
     - id: pool_temp
       type: u1
       doc: The temperature of the pool in chosen units.
     - id: spa_temp
       type: u1
       doc: The temperature of the spa in chosen units.
     - id: unknown_heater_preference
       type: b4
     - id: pool_heater_preference
       type: b2
       enum: heater_preference
     - id: spa_heater_preference
       type: b2
       enum: heater_preference
     - id: unknown3
       size: 1
     - id: air_temperature
       type: u1
       doc: The temperature of air from thermometer in the chosen units.
     - id: solar_temperature
       type: u1
       doc: The ? temperature
     - id: unknown4
       size: 2
     - id: heater_mode
       type: u1
     - id: unknown5
       size: 3
     - id: auto_adjust_dst
       type: u1
       doc: 1 if we should auto adjust for daylight savings time - pretty sure this is wrong and is 2 more bits over
     - id: unknown6
       size: 2
    enums:
      temperature_units:
        0x00: farenheit
        0x04: celcius
      heater_preference:
        0: off
        1: heater_only # I am not sure whether this means only or preferred.
        2: solar_preferred
        3: solar_only
      run_mode: # FIX-ME(ssilver) in the sense of this right?
        0: auto
        1: service
  pump_remote_control:
    seq:        
      - id: status
        type: u1
        enum: on_or_off
    enums:
      on_or_off:
        0xff: on
        0x00: off
  pump_power:
    seq:        
      - id: status
        type: u1
        enum: on_or_off
    enums:
      on_or_off:
        0x0a: on
        0x04: off
  pump_data:
    seq:
     - id: pump_data_type
       type: u1
       doc: Tells us the type of the data structure we're going to see.
     - id: body
       size: _parent.size - 1 # Set the size of the switch to exclude the pump_data_type bit, above.
       type:
          switch-on: pump_data_type
          cases:
            2: pump_data_rpm
            7: pump_ack
  pump_data_rpm:
    seq:
      - id: rpm
        type: u2
  pump_ack:
    seq:
      - id: ack
        type: u1
  heat_set_points:
      seq:        
        - id: pool_temperature
          type: u1
          doc: Current pool temperature
        - id: spa_temperature
          type: u1
          doc: Current spa temperature
        - id: air_temperature
          type: u1
          doc: Current air temperature.
        - id: pool_setpoint
          type: u1
          doc: Turn on the pool heater if the heat falls below this.
        - id: spa_setpoint
          type: u1
          doc: Turn on the spa heater if the heat falls below this.
        - id: spa_heat_mode
          type: b2
          doc: Set to non-zero if the spa heater is on.
        - id: pool_heat_mode
          type: b2
          doc: Set to non-zero if the pool heater is on.
        - id: unknown0
          size: 2
        - id: solar_temperature
          type: u1
        - id: unknown1
          size: 4
  chlorinator_unknown0:
    seq:
     - id: status
       type: u1
  chlorinator_status:
    seq:
     - id: output_spa_percent
       type: u1
     - id: output_percent
       type: u1
     - id: unknown0
       type: u1
     - id: salt_ppm
       type: u1
     - id: status
       type: u1
     - id: unknown1
       type: u1
     - id: name
       type: str
       encoding: ASCII
       size: 16
  controller_date:
    seq:
     - id: hour
       type: u1
       doc: Hours of current time.
     - id: minute
       type: u1
       doc: Minutes of current time.
     - id: day_of_week
       type: u1
       enum: day_of_week
     - id: date
       type: u1
     - id: month
       type: u1
     - id: year
       type: u1
       doc: 2 digit year - apparently we're angling for a year 2100 problem...
     - id: unknown
       type: u1
     - id: auto_adjust_dst
       type: u1
       doc: Should we auto adjust to Dst.
    enums:
       day_of_week:
         1: sunday 
         2: monday
         4: tuesday
         8: wednesday
         16: thursday
         32: friday
         64: saturday
  pump_status_request_or_response:
   seq:
    - id: pump_status_response
      type: pump_status_response
      if: _parent.size > 0
      doc: This is a response iff the message length is > 0, otherwise it's a request.
  pump_status_response:
    seq:
     - id: run
       type: u1
     - id: mode
       type: u1
     - id: drive_state
       type: u1
     - id: power_in_watts
       type: u2
     - id: rpm
       type: u2
     - id: gpm
       type: u1
     - id: ppc
       type: u1
     - id: unknown0
       type: u1
     - id: error
       type: u1
     - id: unknown1
       type: u1
     - id: unknown2
       type: u1
     - id: time
       type: time
  time:
    seq:
      - id: hour
        type: u1
      - id: minute
        type: u1