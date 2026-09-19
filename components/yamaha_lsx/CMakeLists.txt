idf_component_get_property(bt_dir bt COMPONENT_DIR)

idf_component_register(
    SRCS "yamaha_lsx.cpp"
    INCLUDE_DIRS "." "${bt_dir}/host/bluedroid/api/include/api"
    REQUIRES "bt" "esp_common" "nvs_flash"
)
