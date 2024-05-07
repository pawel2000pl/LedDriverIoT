"use strict";

function dumpConfig() {
    return {
        "wifi": {
            "hostname": $id('wifi-hostname').value,
            "sta_priority": $id('sta-priority-list').getValues(),
            "access_point": {
                "enabled": $id('enable-ap').checked,
                "ssid": $id('ap-ssid').value,
                "password": $id('ap-password').value,
                "address": $id('ap-address').value,
                "gateway": $id('ap-gateway').value,
                "subnet": $id('ap-subnet').value
            },
        },
        "channels": {
            "webMode": $id('web-mode-colorspace').value,
            "knobMode": $id('knobs-mode-colorspace').value
        },
        "filters": {
            "inputFilters": {
                "hue": $id('input-hue').getValues(),
                "saturation": $id('input-saturation').getValues(),
                "value": $id('input-value').getValues(),
                "lightness": $id('input-lightness').getValues(),
                "red": $id('input-red').getValues(),
                "green": $id('input-green').getValues(),
                "blue": $id('input-blue').getValues(),
                "white": $id('input-white').getValues()
            },
            "globalInputFilters": $id('input-common-filter').getValues(),
            "globalOutputFilters": $id('output-common-filter').getValues(),
            "outputFilters": {
                "red": $id('output-red').getValues(),
                "green": $id('output-green').getValues(),
                "blue": $id('output-blue').getValues(),
                "white": $id('output-white').getValues()
            }
        },
        "hardware": {
            "potentionemterConfiguration": $id('input-table').getValues(),
            "transistorConfiguration": $id('output-table').getValues(),
            "bias": {
                "up": Number($id('low-bias-input').value),
                "down": Number($id('high-bias-input').value)
            },
            "knobActivateDelta": Number($id('knob-activate-input').value),
            "enbleWhiteKnob": $id('activate-white-knob').checked
        }
    };
}


function fillConfig(config) {
    $id('wifi-hostname').value = config.wifi.hostname;
    $id('sta-priority-list').setValues(config.wifi.sta_priority);
    $id('enable-ap').checked = config.wifi.access_point.enabled;
    $id('ap-ssid').value = config.wifi.access_point.ssid;
    $id('ap-password').value = config.wifi.access_point.password;
    $id('ap-address').value = config.wifi.access_point.address;
    $id('ap-gateway').value = config.wifi.access_point.gateway;
    $id('ap-subnet').value = config.wifi.access_point.subnet;

    $id('web-mode-colorspace').value = config.channels.webMode;
    $id('knobs-mode-colorspace').value = config.channels.knobMode;

    $id('input-hue').setValues(config.filters.inputFilters.hue);
    $id('input-saturation').setValues(config.filters.inputFilters.saturation);
    $id('input-value').setValues(config.filters.inputFilters.value);
    $id('input-lightness').setValues(config.filters.inputFilters.lightness);
    $id('input-red').setValues(config.filters.inputFilters.red);
    $id('input-green').setValues(config.filters.inputFilters.green);
    $id('input-blue').setValues(config.filters.inputFilters.blue);
    $id('input-white').setValues(config.filters.inputFilters.white);
    $id('input-common-filter').setValues(config.filters.globalInputFilters);
    $id('output-common-filter').setValues(config.filters.globalOutputFilters);
    $id('output-red').setValues(config.filters.outputFilters.red);
    $id('output-green').setValues(config.filters.outputFilters.green);
    $id('output-blue').setValues(config.filters.outputFilters.blue);
    $id('output-white').setValues(config.filters.outputFilters.white);

    $id('input-table').setValues(config.hardware.potentionemterConfiguration);
    $id('output-table').setValues(config.hardware.transistorConfiguration);

    $id('low-bias-input').value = config.hardware.bias.up;
    $id('high-bias-input').value = config.hardware.bias.down;

    $id('knob-activate-input').value = config.hardware.knobActivateDelta;
    $id('activate-white-knob').checked = config.hardware.enbleWhiteKnob;
}

$id('save-settings-btn').onclick = ()=>{config = dumpConfig(); saveConfig();};
$id('revert-settings-btn').onclick = ()=>{configPromise = refreshConfig().then(()=>fillConfig(config));};
$id('default-settings-btn').onclick = async ()=>{
    if (confirm("All settings "+"(wifi credentials, filters, etc)"+" will be reverted to default values."+" "+"Are you sure you want to continue?")) {
        await refreshConfig(true);
        await saveConfig();
    }
};
$id('export-settings-btn').onclick = exportConfigToJSON;
$id('import-settings-btn').onclick = async ()=>{
    try {
        const newConfig = await importConfigFromFile();
        if (newConfig === null) return;
        fillConfig(newConfig);
        config = newConfig;
        alert("Configuration imported."+" "+"Now you can save them.");
    } catch {
        fillConfig(config);
        alert('Error ocurred.'+' Invalid configuration file.');
    }
};
