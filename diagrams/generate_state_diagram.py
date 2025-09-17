import graphviz

# Create a new directed graph for the state machine
dot = graphviz.Digraph('TrafficLightStateDiagram', comment='Traffic Light State Machine')
dot.attr(rankdir='TB', splines='curved')
dot.attr('node', shape='box', style='rounded,filled', fillcolor='lightgoldenrodyellow')
dot.attr('edge', fontsize='10')

# Define the lanes and their sequence
lanes = ["Utara", "Timur", "Selatan", "Barat"]

# Create the states and transitions
for i in range(len(lanes)):
    current_lane = lanes[i]
    next_lane = lanes[(i + 1) % len(lanes)]

    # State: Green light for the current lane
    green_state_id = f'G_{current_lane}'
    green_state_label = f'Lampu Hijau: {current_lane}\n(Lainnya Merah)'
    dot.node(green_state_id, green_state_label, fillcolor='lightgreen')

    # State: Yellow light for the current lane
    yellow_state_id = f'Y_{current_lane}'
    yellow_state_label = f'Lampu Kuning: {current_lane}\n(Lainnya Merah)'
    dot.node(yellow_state_id, yellow_state_label, fillcolor='lightyellow')

    # Transition from Green to Yellow
    dot.edge(green_state_id, yellow_state_id, label='Setelah 20 detik')

    # Transition from Yellow to the next lane's Green
    next_green_state_id = f'G_{next_lane}'
    dot.edge(yellow_state_id, next_green_state_id, label='Setelah 5 detik')

# Set the initial state (optional, for clarity)
dot.node('start', 'Mulai', shape='ellipse', fillcolor='lightblue')
dot.edge('start', f'G_{lanes[0]}', label='Inisialisasi')


# Render the graph
try:
    dot.render('state_diagram', format='svg', view=False, cleanup=True)
    print("State machine diagram 'state_diagram.svg' generated successfully.")
except Exception as e:
    print(f"An error occurred during rendering: {e}")
