import schemdraw
import schemdraw.elements as elm

# --- Pin Configuration (from lampu_lalin_esp32.ino) ---
PIN_CONFIG = {
    "Utara":   {'R': 4,  'Y': 25, 'G': 26},
    "Timur":   {'R': 17, 'Y': 27, 'G': 32},
    "Selatan": {'R': 16, 'Y': 5,  'G': 18},
    "Barat":   {'R': 19, 'Y': 23, 'G': 33},
}
OLED_PINS = {'SDA': 21, 'SCL': 22}

# --- ESP32 DevKit V1 Pinout ---
# We represent the ESP32 as two headers
left_pins = [
    ("EN", "EN"), ("VP", "SVP"), ("VN", "SVN"), ("P34", "34"),
    ("P35", "35"), ("P32", "32"), ("P33", "33"), ("P25", "25"),
    ("P26", "26"), ("P27", "27"), ("P14", "14"), ("P12", "12"),
    ("GND", "GND"), ("VIN", "VIN"), ("3V3", "3V3")
]
right_pins = [
    ("GND", "GND"), ("P23", "23"), ("P22", "22"), ("TXD", "TX0"),
    ("RXD", "RX0"), ("P21", "21"), ("P19", "19"), ("P18", "18"),
    ("P5", "5"),   ("P17", "17"), ("P16", "16"), ("P4", "4"),
    ("P2", "2"),   ("P15", "15"), ("GND", "GND")
]

with schemdraw.Drawing(file='wiring_diagram.svg', show=False, unit=2.5) as d:
    d.config(fontsize=10)

    # --- ESP32 Headers ---
    L = elm.Header(rows=len(left_pins), pinspacing=1).at((-4, 0))
    d.add(L)
    for i, (pin_name, pin_num) in enumerate(left_pins):
        pin_label = f'{pin_name} ({pin_num})'
        d.add(elm.Dot(radius=0).at(getattr(L, f'pin{i+1}')).label(pin_label, loc='left', ofst=0.5))

    R = elm.Header(rows=len(right_pins), pinspacing=1).at((4, 0))
    d.add(R)
    for i, (pin_name, pin_num) in enumerate(right_pins):
        pin_label = f'({pin_num}) {pin_name}'
        d.add(elm.Dot(radius=0).at(getattr(R, f'pin{i+1}')).label(pin_label, loc='right', ofst=0.5))

    d.add(elm.Label("ESP32 DevKit V1").at((0, L.pin1.y + 2)))

    # --- Ground and VCC Busses ---
    gnd_bus_y = -22
    d.add(elm.Line(l=30).at((-15, gnd_bus_y)).label("GND Bus", loc="bot"))
    d.add(elm.Ground().at((-15, gnd_bus_y)))
    # Connect ESP32 grounds to bus
    d.add(elm.Line().at(L.pin13).to((L.pin13.x+0.5, L.pin13.y)).to((L.pin13.x+0.5, gnd_bus_y)).to((-15, gnd_bus_y)))
    d.add(elm.Dot(at=L.pin13))

    vcc_bus_y = L.pin1.y + 3
    d.add(elm.Line(l=30).at((-15, vcc_bus_y)).label("3.3V Bus", loc="top"))
    d.add(elm.Vdd().at((-15, vcc_bus_y)).left())
    # Connect ESP32 3.3V to bus
    d.add(elm.Line().at(L.pin15).to((L.pin15.x+0.5, L.pin15.y)).to((L.pin15.x+0.5, vcc_bus_y)).to((-15, vcc_bus_y)))
    d.add(elm.Dot(at=L.pin15))

    # --- Components ---

    # OLED Display
    oled_pos = (12, 10)
    oled = elm.Ic(size=(2,4)).at(oled_pos).label("OLED SSD1306", "top")
    oled.pin(pin='GND', side='L', slot='1/4', anchorname='GND')
    oled.pin(pin='VCC', side='L', slot='2/4', anchorname='VCC')
    oled.pin(pin='SCL', side='L', slot='3/4', anchorname='SCL')
    oled.pin(pin='SDA', side='L', slot='4/4', anchorname='SDA')
    d.add(oled)
    d.add(elm.Line().at(oled.SDA).to(R.pin6)) # GPIO 21
    d.add(elm.Dot(at=R.pin6))
    d.add(elm.Line().at(oled.SCL).to(R.pin3)) # GPIO 22
    d.add(elm.Dot(at=R.pin3))
    d.add(elm.Line().at(oled.VCC).to((oled.VCC.x-1, oled.VCC.y)).to((oled.VCC.x-1, vcc_bus_y)).to((-15, vcc_bus_y)))
    d.add(elm.Line().at(oled.GND).to((oled.GND.x-1, oled.GND.y)).to((oled.GND.x-1, gnd_bus_y)).to((-15, gnd_bus_y)))


    # Traffic Lights
    lane_x_start = -12
    lane_y_start = -5

    # Create a mapping from GPIO number (as string) to the header pin object
    pin_map = {}
    for i, p in enumerate(left_pins):
        pin_map[p[1]] = getattr(L, f'pin{i+1}')
    for i, p in enumerate(right_pins):
        pin_map[p[1]] = getattr(R, f'pin{i+1}')

    for i, (lane, pins) in enumerate(PIN_CONFIG.items()):
        group_x = lane_x_start + i * 8
        d.add(elm.Label(lane).at((group_x + 1.5, lane_y_start + 1)))

        for j, (color_code, color_name) in enumerate([('R', 'red'), ('Y', 'yellow'), ('G', 'green')]):
            pin_num_str = str(pins[color_code])
            gpio_pin = pin_map[pin_num_str]

            led_x = group_x + j * 2
            led = elm.LED(color=color_name).at((led_x, lane_y_start - 2)).down()
            res = elm.Resistor().down().label("220Ω")
            d.add(led)
            d.add(res)
            d.add(elm.Line().at(res.end).to((res.end.x, gnd_bus_y)).to((-15, gnd_bus_y)))

            d.add(elm.Dot(at=gpio_pin))
            d.add(elm.Line().at(led.start).to(gpio_pin))

    d.draw()

print("Wiring diagram 'wiring_diagram.svg' generated successfully.")
