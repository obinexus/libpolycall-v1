// main.js
const DOPAdapter = require('./adapter/DOPAdapter');
const logic = require('./components/ButtonLogic');

const renderFunc = require('./render/renderFunc');
const renderOOP = require('./render/renderOOP');

const adapter = new DOPAdapter(logic);

// Functional version
const FuncButton = adapter.toFunctional();
renderFunc(FuncButton);

// OOP version
const OOPButton = adapter.toOOP();
renderOOP(OOPButton);
