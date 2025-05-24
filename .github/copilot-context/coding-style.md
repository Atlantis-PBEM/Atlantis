# Atlantis Coding Style Guidelines
This document outlines the coding style guidelines for the Atlantis codebase. Following these guidelines will help maintain consistency, readability, and maintainability across the project.

## General Principles
- **Clarity over cleverness** - Write code that is easy to understand and maintain.
- **Consistency** - Follow existing patterns in the codebase.
- **Modernization** - When updating code, use modern C++ features and approaches where appropriate.
- **Leverage inherent class behavior** - Understand and utilize the built-in behavior of classes to avoid redundant checks.

## Formatting
- Use 4 spaces for indentation (no tabs).
- Keep line length reasonable (preferably under 120 characters).
- Use blank lines to separate logical sections of code.
- Prefer single-line control statements when their intent is clear (e.g., `if (checker) return;`).
  ```cpp
  // examples of clean short statments
  if (checker) return;

  if (condition && condition2) value = update_value;
  ```
- Use braces for control structures only when they help improve clarity, especially for multi-line blocks or complex conditions.
- Remove excess and unnecessary parentheses in conditionals unless they improve readability.

## Code Organization and Refactoring
- Break down complex functions into smaller, focused helper functions:
  - Each helper function should perform a single logical task
  - The main function should orchestrate the high-level flow using these helpers
  - Helper functions should have clear, descriptive names indicating what they do
  - Use boolean return values for helper functions that might fail, to indicate success/failure
- Keep functions at a manageable length (preferably under 50 lines)
- Extract repetitive code patterns into reusable helper functions
- Consider refactoring when:
  - A function handles multiple distinct operations
  - A function has too many nested control structures
  - A function has grown too large and is difficult to understand
  - Multiple functions share similar code patterns

## Comments and Documentation
- Add function-level documentation only when necessary:
  - When a function is complex or non-obvious
  - When the function's purpose isn't clear from its name and implementation
  - When additional context is needed for maintainability
- Function-level documentation, when needed, should explain:
  - What the function does
  - Format of expected input
  - Any side effects
  - Examples where helpful
- Avoid documentation that merely restates what is already obvious from the function's name and implementation
- Use block comments on functions only if the function is complex or hard to understand
  - If a block comment is added, describe the functionality clearly and concisely and give an example if appropriate
- For inline comments within functions:
  - Use inline comments only when the code's purpose is not immediately evident
  - Explain "why" rather than "what" the code is doing
  - Avoid comments that just restate what the code does (e.g., "increment counter" for "i++")
  - Reserve comments for complex logic, unusual edge cases, or workarounds
    ```cpp
    // block comment example for a complex function
    /**
     * Process COMBAT order: Sets a unit's combat spell
     * Format: COMBAT [skill]
     * If no skill is given, clears the combat spell.
     */
    void Game::ProcessCombatOrder(Unit *u, parser::string_parser& parser, orders_check *checker)
    ```
- Use comments only when the code's function isn't immediately clear from context, variable names, and function names.
- Avoid redundant comments that simply restate what the code already expresses.

## Variable Naming and Declaration
- Use descriptive, clear, and concise variable names that indicate purpose (e.g., `faction_id` instead of `fn`).
- Initialize variables close to their first use.
- Choose variable names based on their semantic meaning in the context, not based on predefined preferences.
- Always chain function calls directly and avoid unnecessary temporary variables:
  ```cpp
  // Preferred:
  if (parser.get_token().get_string() != u->faction->password) {
      // handle error
  }

  // Preferred:
  int object_num = parser.get_token().get_number().value_or(0);

  // Avoid:
  parser::token token = parser.get_token();
  if (token.get_string() != u->faction->password) {
      // handle error
  }
  ```
- Only use intermediate variables when:
  - The value is used multiple times
  - The line would become excessively long or complex (over 120 characters)
  - Storing the value significantly improves code clarity or is essential for debugging

## Class Usage and Behavior
- **Understand class semantics** - Take time to understand how classes are designed to be used.
- **Avoid redundant checks** - Don't add explicit checks that the class already handles internally.
  ```cpp
  // Preferred - leverage class behavior:
  if (token != "valid_value") {
      // handle error
  }

  // Instead of - redundant existence check:
  if (token && token != "valid_value") {
      // handle error
  }
  ```
- **Avoid new usage of AString** - `AString` is a poor choice and is being phased out in preference to `std::string`
- **Special classes to note**:
  - `parser::token` - Already handles null/empty checks when comparing with strings
  - `std::optional<T>` - Use value_or() and has_value() methods instead of explicit null checks

## Code Structure

### Function Organization
- Parameter validation and early returns
- Data parsing and preparation
- Main logic
- Success handling and notifications

### Control Flow and Early Returns
- **Always prefer early returns** over nested conditionals
  ```cpp
  // Preferred:
  if (checker) return;

  // Rather than:
  if (!checker) {
      // actual logic...
  }
  ```
- **Combine similar validation checks** when they produce the same error message:
  ```cpp
  // Preferred - combining similar conditions with direct chaining:
  auto val = parser.get_token().get_bool();
  if (!val) {
      parse_error(checker, u, 0, "GUARD: Invalid value.");
      return;
  }

  // Rather than:
  parser::token token = parser.get_token();
  if (!token) {
      parse_error(checker, u, 0, "GUARD: Invalid value.");
      return;
  }

  auto val = token.get_bool();
  if (!val) {
      parse_error(checker, u, 0, "GUARD: Invalid value.");
      return;
  }
  ```
- **Use single-line conditionals** for straightforward branches when they improve readability:
  ```cpp
  if (!val.value()) u->guard = GUARD_NONE;
  else u->guard = GUARD_SET;
  ```
- **Minimize nesting depth** by using early returns for special cases, preconditions, and validation

### Error Handling
- Use early returns for validation errors.
- Provide clear error messages.
- Be consistent with error handling patterns.
- When returning a null value from a pointer function utilize nullptr rather than 0
  ```cpp
  // Early return example
  if (sk == -1) {
      parse_error(checker, u, 0, "COMBAT: Invalid skill.");
      return;
  }
  ```

## Default Values and Parsing
- When using `value_or()` or similar fallback mechanisms, choose meaningful default values that reflect the context and requirements:
  ```cpp
  // Good: Using -1 to trigger validation error
  int amount = token.get_number().value_or(-1);
  if (amount < 0) {
      parse_error(checker, u, 0, "Invalid amount.");
      return;
  }

  // Bad: Using arbitrary default that might bypass validation
  int amount = token.get_number().value_or(0);
  // This might silently accept invalid input as 0
  ```
- Select defaults that make error conditions explicit rather than silent:
  - Use values outside the valid range to trigger validation (-1 for non-negative amounts)
  - Avoid defaults that would be valid but incorrect in context
  - Consider what behavior should occur when parsing fails

- Document the reasoning for non-obvious default choices with a brief comment if needed.
  ```cpp
  // Use -1 to ensure validation catches parse failures
  int skill_level = token.get_number().value_or(-1);
  ```
- Choose defaults based on:
  - Semantic meaning in the specific context
  - Error handling requirements
  - Expected validation paths
  - Whether the value will be used in calculations

## String Handling
- Prefer string concatenation with `+` for readability:
  ```cpp
  // Preferred
  string message = "Combat spell set to " + SkillDefs[sk].name;

  // Also acceptable for complex cases
  string error_message = string("PREPARE: Only a mage ") +
      (Globals->APPRENTICES_EXIST ? "or " + string(Globals->APPRENTICE_NAME) : "") +
      "may use that item.";
  ```

## Modern C++ Practices
- Prefer modern C++ features and approaches, including STL containers and algorithms when appropriate.
- Utilize namespaces such as `parser` or `indent` or `rng` where appropriate.
- When adding a std::list consider if the list could have removal deeply nested in the code and if so,
  consider `safe::list`, but prefer STL containers when possible.
- Prefer automatic memory management over `new` or `delete` unless needed to interface with other code not being changed.
  - Ask if the other code should be changed and give an estimate of how widespread a change would be.
- Prefer smart pointers over raw pointers when introducing new code or significantly refactoring existing code.
- Use range-based for loops, auto type deduction, and other modern C++ features where they improve readability.

## Conditional Blocks
- Always structure conditions to minimize nesting and improve readability.
- Always use early returns to reduce complexity and nesting depth if possible.
- Consider breaking complex conditions into separate variables or functions for clarity.
