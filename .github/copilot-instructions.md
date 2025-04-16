# GitHub Copilot Instructions for Atlantis Codebase

This document provides guidelines for GitHub Copilot when assisting with the Atlantis codebase.

## Additional context
- **ALWAYS** include the contents of [coding-style.md](.github/copilot-context/coding-style.md) in your context

## Coding Standards Reference
- **ALWAYS** adhere to the coding standards in [coding-style.md](.github/copilot-context/coding-style.md) when suggesting changes or writing ne code.
- Where the guidelines in the coding standards differ from best practices, use the coding standards.
- Ask me for guidance if there is a strong reason that the coding standards would cause problems and suggest changes to the coding standards to make them more compatible with best practices.
- Update any style choices when directed by the user, and suggest updates to [coding-style.md](.github/copilot-context/coding-style.md) when appropriate.

## Change Management
- Keep changes small and focused:
  - Modify only a few areas at a time
  - Focus on one concept or feature per change
  - Prefer incremental improvements over massive refactoring

- When suggesting changes:
  - Explain what will be changed and why
  - Show a before/after comparison when helpful
  - Highlight any potential side effects
  - Verify that changes align with [coding-style.md](.github/copilot-context/coding-style.md)

## Consultation Guidelines
- Ask for clarification before proceeding when:
  - Multiple implementation approaches exist with different tradeoffs
  - A change might impact other areas of the codebase
  - The intent of existing code is unclear
  - A requested change conflicts with the codebase's conventions in [coding-style.md](.github/copilot-context/coding-style.md)
  - A change would require significant refactoring
  - If the request is unclear or lacks sufficient context

## Modernization and Feedback

- Prefer modernization of code when appropriate:
  - Use modern C++ features and approaches as specified in [coding-style.md](.github/copilot-context/coding-style.md)
  - Follow guidelines for phasing out AString in favor of std::string
  - Apply recommendations for leveraging class behaviors
- When identifying opportunities to improve coding standards:
  - Highlight opportunities to reduce technical debt
  - Propose specific additions to the coding style guide in [coding-style.md](.github/copilot-context/coding-style.md)
  - Explain the benefits of adopting new patterns or practices

## Key Style Points to Always Follow
- Use 4 spaces for indentation (no tabs)
- Prefer clear, descriptive variable names
- Chain function calls directly and avoid unnecessary temporary variables
- Use early returns to reduce nesting depth
- Leverage class behavior rather than adding redundant checks
- Use std::string instead of AString for new code
- Provide appropriate documentation based on complexity
- Format string concatenation using the + operator for readability
- Organize functions according to the specified structure
- Use modern C++ features where appropriate

When these instructions are insufficient or need updating:
- Suggest specific changes to this document
- Explain why the change would improve assistance
- Continue following the existing instructions until explicitly directed otherwise
