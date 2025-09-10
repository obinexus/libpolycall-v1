// Obix Component Validation Specification

// Core validation interface
interface ValidationConstraint {
    type: string;
    required?: boolean;
    validate: (value: any, context?: Record<string, any>) => boolean;
    errorMessage: string;
  }
  
  // Component Validation Descriptor
  class ComponentValidator {
    private constraints: Record<string, ValidationConstraint> = {};
  
    constructor(validationSpec: Record<string, ValidationConstraint>) {
      this.constraints = validationSpec;
    }
  
    validate(componentProps: Record<string, any>): void {
      Object.entries(this.constraints).forEach(([propName, constraint]) => {
        const propValue = componentProps[propName];
  
        // Check required field
        if (constraint.required && (propValue === undefined || propValue === null)) {
          throw new ValidationError(
            `MISSING_REQUIRED_PROP`,
            `Required prop '${propName}' is missing`,
            'ComponentValidator'
          );
        }
  
        // Skip validation if value is undefined and not required
        if (propValue === undefined) return;
  
        // Type and custom validation
            if (!constraint.validate(propValue, componentProps)) {
              throw new ValidationError(
                `VALIDATION_FAILED`,
                `Validation failed for prop '${propName}': ${constraint.errorMessage}`,
                'ComponentValidator'
              );
            }
      });
    }
  }
  
  // Custom Error for Validation
  class ValidationError extends Error {
    code: string;
    source: string;
  
    constructor(code: string, message: string, source: string) {
      super(message);
      this.name = 'ValidationError';
      this.code = code;
      this.source = source;
    }
  }
  
  // Example Component Validation Specification
  const ItemListValidator = new ComponentValidator({
    title: {
      type: 'string',
      required: true,
      validate: (value) => typeof value === 'string' && value.length >= 3,
      errorMessage: 'Title must be at least 3 characters long'
    },
    items: {
      type: 'array',
      validate: (value) => Array.isArray(value) && value.length <= 10,
      errorMessage: 'Items array must contain 10 or fewer items'
    },
    onItemClick: {
      type: 'function',
      validate: (value: any, props: Record<string, any>) => {
        // Additional context-aware validation
        if (props.disabled && value) {
          return false;
        }
        return typeof value === 'function' || value === undefined;
      },
      errorMessage: 'Invalid click handler for component state'
    }
  });
  
  // Usage Example
  function validateItemListComponent(props: any) {
    try {
      ItemListValidator.validate(props);
      // Proceed with component rendering/processing
      console.log('Component validation successful');
    } catch (error) {
      if (error instanceof ValidationError) {
        // Log or handle validation errors
        console.error(`Validation Failed: ${error.message}`);
        // Potential error reporting or fallback mechanism
        throw error; // Re-throw for upstream handling
      }
      throw error; // Re-throw unexpected errors
    }
  }
  
  // Demonstration of component validation usage
  function exampleUsage() {
    // Valid props
    const validProps = {
      title: 'My List',
      items: ['Item 1', 'Item 2'],
      onItemClick: () => {}
    };
  
    // Invalid props
    const invalidProps = {
      title: 'A', // Too short
      items: new Array(15).fill('Item'), // Too many items
      onItemClick: undefined
    };
  
    try {
      validateItemListComponent(validProps);
      validateItemListComponent(invalidProps); // This will throw an error
    } catch (error) {
      // Error handling logic
    }
  }
  
  // Export for integration with Obix runtime
  export {
    ComponentValidator,
    ValidationError,
    ItemListValidator,
    validateItemListComponent
  };