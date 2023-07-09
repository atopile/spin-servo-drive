# servo drive

An inexpensive servo driver for brushless motors, compilable with atopile.

## Developer guide

### Adding components

For now, manufacturing is with JLCPCB.

[easyeda2kicad](https://github.com/uPesy/easyeda2kicad.py) is a great tool to pull from their footprint library.

1. Install with with `pip install easyeda2kicad`
2. `cd <here>`
3. Add a component with: `easyeda2kicad --full --lcsc_id=C77540 --output elec/lib/lib`
