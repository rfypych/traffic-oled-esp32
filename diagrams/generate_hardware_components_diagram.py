import graphviz

# Create a new directed graph
dot = graphviz.Digraph('HardwareComponentDiagram', comment='Hardware Component Diagram for ESP32 Traffic Light')
dot.attr(rankdir='LR', splines='ortho', concentrate='true')
dot.attr('node', shape='record', style='rounded,filled', fillcolor='lightblue')

# Central component
dot.node('ESP32', 'ESP32 DevKit V1')

# Peripheral components
dot.node('OLED', 'OLED Display\n(SSD1306)')
dot.node('LANE_N', 'Traffic Light: Utara')
dot.node('LANE_E', 'Traffic Light: Timur')
dot.node('LANE_S', 'Traffic Light: Selatan')
dot.node('LANE_W', 'Traffic Light: Barat')

# Power connections
dot.attr('edge', style='dashed', arrowhead='none', fontsize='10')
dot.edge('ESP32', 'OLED', label='3.3V, GND')

# Data connections
dot.attr('edge', style='solid', arrowhead='normal')
dot.edge('ESP32', 'OLED', label='I2C\nSDA: GPIO 21\nSCL: GPIO 22')
dot.edge('ESP32', 'LANE_N', label='R: GPIO 4\nY: GPIO 25\nG: GPIO 26')
dot.edge('ESP32', 'LANE_E', label='R: GPIO 17\nY: GPIO 27\nG: GPIO 32')
dot.edge('ESP32', 'LANE_S', label='R: GPIO 16\nY: GPIO 5\nG: GPIO 18')
dot.edge('ESP32', 'LANE_W', label='R: GPIO 19\nY: GPIO 23\nG: GPIO 33')


# Render the graph
try:
    dot.render('hardware_components', format='svg', view=False, cleanup=True)
    print("Hardware component diagram 'hardware_components.svg' generated successfully.")
except Exception as e:
    print(f"An error occurred during rendering: {e}")
