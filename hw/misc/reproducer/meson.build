reproducer_ss = ss.source_set()

reproducer_ss.add(files(
  'remote-device.c',
  'trigger-device.c',
  'mapper-device.c',
  'reproducer-soc.c',
  'reproducer-board.c',
))

system_ss.add_all(when: 'CONFIG_REPRODUCER', if_true: reproducer_ss)