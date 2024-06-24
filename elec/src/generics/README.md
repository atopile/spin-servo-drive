# Generics

## Overview

Standard libaries for use in your projects. These include:

- **Resistors**
- **Capacitors**
- **Inductors**
- **Diodes**
- **Transistors**
- **Interfaces**

## Installation
Run in the terminal:

```ato install generics``

Add to your ato project:

- Resistors   ```import Resistor from "generics/resistor.ato```
- Capacitors  ```import Capacitor from "generics/capacitor.ato```
- Inductors   ```import Inductor from "generics/inductor.ato```
- Interfaces  ```import Interface from "generics/interface.ato```
- etc...

## Features
### Resistors
The standard libary includes several thousand resistor options. In your ato project, simply specify any requirements and the best resistor will be automatically selected.
Example:

```import Resistor from "generics/resistor.ato"```
```myResistor = new Resistor```
```myResistor.value = 1000```
```myResistor.footprint = "R0402"```
```myResistor.tolerance = 10```

The solver will find a resistor that has a resistance between 900 and 1100 ohms accounting for tolerance, and a footprint of R0402.

### Capacitors
Similar to resistors, just specify the value and footprint and the solver will find the best capacitor for your application.

### Inductors
Currently not supported for automatic selection, but you can install any inductor you like using '''ato install''' and then use it in your project.

We strongly reccomend using subclassing for new components, something like this:

```import Inductor from "generics/inductor.ato"```
```component MyInductor from Inductor:```
```    value = 1000```
```    footprint = "L0402" ```

Then you can use it in your project like this:

```import MyInductor from "myInductor.ato"```
```myInductor = new MyInductor```
```myInductor.value = 1000```
```myInductor.footprint = "L0402"```

### Interfaces
Interfaces standardize the way you connect to modules. By implementing an interface from the standard libary, you can be sure that your module will be compatible with any other module that implements the same interface. This makes it easy to share modules between projects and be sure that they will work together.

## Contributing

We welcome contributions and suggestions to improve this module. Please submit your contributions as pull requests on our GitHub repository.

## License

This package documentation is provided under the [MIT License](#).

## Contact

For further inquiries or support, please contact us at [email@example.com](mailto:email@example.com).
