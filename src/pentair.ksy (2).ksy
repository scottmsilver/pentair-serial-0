meta:
  id: pentair
  file-extension: pentair.ksy
  endian: be
seq:
  - id: message
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
     - id: stuff
       type: u1
       repeat: until
       repeat-until: _ == 165
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
      - id: command_type
        type: u1
        doc: type of command
      - id: size
        type: u1
        doc: size of command message
      - id: body
        size: size
        type:
          switch-on: command_type
          cases:
            2: controller_status
  controller_status:
    seq:
     - id: hour
       type: u1
       doc: hour of current time
     - id: minute
       type: u1
       doc: minute of current time
     - id: equip1
       type: u1
       doc: equipment 1 - fixme!
     - id: equip2
       type: u1
       doc: equipment 2 - fixme!
     - id: equip3
       type: u1
       doc: equipment 3 - fixme!
     - id: unknown1
       size: 4
     - id: temperature_units
       type: u1
       enum: temperature_units
       doc: equipment 1 - fixme!
     - id: valves
       type: u1
       doc: not sure
     - id: unknown2
       size: 2
     - id: unknown3
       type: u1
       doc: something to do with heat
     - id: pool_temp
       type: u1
       doc: degrees of pool in chosen units.
     - id: spa_temp
       type: u1
       doc: degrees of spa in chosen units.
     - id: heater_active
       type: u1
       doc: is the heater on?
     - id: unknown4
       size: 1
     - id: air_temperature
       type: u1
       doc: temperature of air from thermometer
     - id: solar_temperature
       type: u1
       doc: solar temperature
     - id: unknown5
       size: 2
     - id: heater_mode
       type: u1
     - id: unknown6
       size: 3
     - id: auto_adjust_dst
       type: u1
       doc: 1 if we should auto adjust for daylight savings time
    enums:
      temperature_units:
        0: farenheit
        4: celcius

              
        
