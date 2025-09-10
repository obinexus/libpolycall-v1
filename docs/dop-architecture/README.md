# DOP Adapter

This project demonstrates a simple **Data-Oriented Programming Adapter (DOP Adapter)** that bridges functional and object-oriented programming paradigms for UI component logic.

## Overview

The DOP Adapter takes a UI component defined in a declarative format and provides two adapters:

* A **Functional Adapter**, which renders the component using a function-based style.

* An **Object-Oriented Adapter**, which renders the component using a class-based style.

The example component used here is a simple `Button` that toggles between "ON" and "OFF".

## File Structure

```
├── components
│   └── ButtonLogic.js     # Contains the declarative logic for the Button component
├── render
│   ├── renderFunc.js      # Functional rendering of a component
│   └── renderOOP.js       # OOP-style rendering of a component
├── adapter
│   └── DOPAdapter.js      # Core adapter logic for converting to Functional and OOP components
└── main.js                # Entry point to demonstrate both rendering methods
```

## How It Works

### Button Logic

`ButtonLogic.js` defines a component as a declarative object with:

* `state`: The initial state (`clicked: false`)

* `actions`: Methods to mutate state (`toggle`)

* `render`: A render function that displays the button with the current state

### Adapter

`DOPAdapter.js` takes a component logic object and:

* `toFunctional()`: Returns a stateless functional component

* `toOOP()`: Returns a class with encapsulated state and methods

### Renderers

* `renderFunc.js`: Executes and logs the output of the functional component.

* `renderOOP.js`: Instantiates the class and logs the output of its render method.

## Run the Demo

```bash
node main.js
```

### Sample Output

```
[Functional Render]: <button>OFF</button>
[OOP Render]: <button>OFF</button>
```

## Future Work

* Implement interaction to actually toggle state

* Extend to more complex components

* Introduce reactive data binding or DOM integration

## License

MIT
