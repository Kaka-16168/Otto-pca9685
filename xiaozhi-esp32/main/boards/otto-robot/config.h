{
  "target": "esp32s3",
  "builds": [
    {
      "name": "otto-robot",
      "sdkconfig_append": [
        "CONFIG_BOARD_TYPE_OTTO_ROBOT=y",
        "CONFIG_USE_CUSTOM_WAKE_WORD=y",
        "CONFIG_CUSTOM_WAKE_WORD=\"你好小智\"",
        "CONFIG_SR_MN_CN_MULTINET7_QUANT=y"
      ]
    }
  ]
}
