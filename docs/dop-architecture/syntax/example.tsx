import React, { useState, useMemo } from 'react';

// Type definitions for prop validation
interface ItemListProps {
  title: string;
  items?: string[];
  onItemClick?: (item: string) => void;
  disabled?: boolean;
}

// Custom validation Higher-Order Component (HOC)
const withValidation = <P extends object>(
  WrappedComponent: React.ComponentType<P>, 
  validationRules: {
    [K in keyof P]?: {
      required?: boolean;
      type?: string;
      validator?: (value: P[K], props: P) => void;
    }
  }
) => {
  return (props: P) => {
    // Validate props before rendering
    useMemo(() => {
      Object.entries(validationRules).forEach(([propName, rule]) => {
        const propValue = props[propName as keyof P];
        
        // Check if prop is required
        if (rule.required && (propValue === undefined || propValue === null)) {
          throw new Error(`Required prop '${propName}' is missing`);
        }

        // Type validation
        if (rule.type && propValue !== undefined) {
          const typeCheck = {
            'string': () => typeof propValue !== 'string',
            'number': () => typeof propValue !== 'number',
            'boolean': () => typeof propValue !== 'boolean',
            'array': () => !Array.isArray(propValue),
            'object': () => typeof propValue !== 'object' || propValue === null,
            'function': () => typeof propValue !== 'function'
          }[rule.type];

          if (typeCheck && typeCheck()) {
            throw new Error(`Prop '${propName}' must be of type ${rule.type}`);
          }
        }

        // Custom validator
        if (rule.validator && propValue !== undefined) {
          try {
            rule.validator(propValue as P[keyof P], props);
          } catch (error) {
            throw new Error(`Validation failed for prop '${propName}': ${(error as Error).message}`);
          }
        }
      });
    }, [props]);

    // Render the wrapped component if validation passes
    return <WrappedComponent {...props} />;
  };
};

// Example component with validation
const ItemList: React.FC<ItemListProps> = ({ title, items = [], onItemClick }) => {
  return (
    <div className="item-list">
      <h2>{title}</h2>
      <ul>
        {items.map((item, index) => (
          <li 
            key={index} 
            onClick={() => onItemClick && onItemClick(item)}
            className="item"
          >
            {item}
          </li>
        ))}
      </ul>
    </div>
  );
};

// Apply custom validation
const ValidatedItemList = withValidation(ItemList, {
  title: {
    required: true,
    type: 'string',
    validator: (value) => {
      if (value.length < 3) {
        throw new Error('Title must be at least 3 characters long');
      }
    }
  },
  items: {
    type: 'array',
    validator: (value) => {
      if (value && value.length > 10) {
        throw new Error('Maximum of 10 items allowed');
      }
    }
  },
  onItemClick: {
    type: 'function',
    validator: (value, props) => {
      if (props.disabled && value) {
        throw new Error('Disabled components cannot have click handlers');
      }
    }
  }
});

// Demonstration component
const ValidationDemo: React.FC = () => {
  const [items, setItems] = useState<string[]>(['Apple', 'Banana']);
  const [title, setTitle] = useState('Fruit List');

  const handleAddItem = () => {
    if (items.length < 10) {
      setItems([...items, `Item ${items.length + 1}`]);
    }
  };

  return (
    <div className="validation-demo">
      <h1>Component Validation Demonstration</h1>
      
      {/* Validated Component Usage */}
      <ValidatedItemList 
        title={title}
        items={items}
        onItemClick={(item) => console.log(`Clicked: ${item}`)}
      />

      {/* Controls to demonstrate validation */}
      <div className="controls">
        <input 
          type="text"
          value={title}
          onChange={(e) => setTitle(e.target.value)}
          placeholder="Enter list title"
          className="title-input"
        />
        <button 
          onClick={handleAddItem}
          disabled={items.length >= 10}
          className="add-item-btn"
        >
          Add Item
        </button>
      </div>

      {/* Validation Notes */}
      <div className="validation-notes">
        <p>Validation Rules:</p>
        <ul>
          <li>Title must be at least 3 characters</li>
          <li>Maximum 10 items allowed</li>
          <li>Disabled components cannot have click handlers</li>
        </ul>
      </div>
    </div>
  );
};

export default ValidationDemo;