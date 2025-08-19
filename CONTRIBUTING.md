# Contributing to Bitcoin Decentral (BTCD)

Thank you for your interest in contributing to Bitcoin Decentral! This document outlines the guidelines and processes for contributing to the project.

## Code of Conduct

We are committed to providing a welcoming and inclusive environment for all contributors. Please be respectful and professional in all interactions.

## Development Workflow

### Getting Started

1. Fork the repository
2. Clone your fork locally
3. Create a feature branch from `main`
4. Make your changes
5. Test your changes thoroughly
6. Submit a pull request

### Branch Naming Convention

- `feature/description` - New features
- `bugfix/description` - Bug fixes
- `docs/description` - Documentation updates
- `refactor/description` - Code refactoring

## Coding Standards

### C++ Style Guide

We follow a modified version of the Bitcoin Core coding standards:

- **Indentation:** 4 spaces (no tabs)
- **Line Length:** Maximum 120 characters
- **Naming Conventions:**
  - Variables: `snake_case`
  - Functions: `CamelCase`
  - Classes: `CamelCase`
  - Constants: `UPPER_SNAKE_CASE`
  - Namespaces: `lowercase`

### Code Quality Requirements

- All code must compile without warnings
- New features require comprehensive unit tests
- Code coverage should not decrease
- All tests must pass before merging
- Use meaningful variable and function names
- Add comments for complex logic

### Commit Message Format

```
<type>(<scope>): <subject>

<body>

<footer>
```

**Types:**
- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation changes
- `style`: Code style changes
- `refactor`: Code refactoring
- `test`: Adding or updating tests
- `chore`: Maintenance tasks

**Example:**
```
feat(consensus): implement CTOR transaction ordering

Add canonical transaction ordering to improve block compression
and enable Xthinner algorithm implementation.

Closes #123
```

## Pull Request Process

### Before Submitting

1. Ensure your code follows the style guidelines
2. Run all tests and ensure they pass
3. Update documentation if necessary
4. Rebase your branch on the latest `main`
5. Squash commits if appropriate

### PR Description Template

```markdown
## Description
Brief description of changes

## Type of Change
- [ ] Bug fix
- [ ] New feature
- [ ] Breaking change
- [ ] Documentation update

## Testing
- [ ] Unit tests added/updated
- [ ] Integration tests added/updated
- [ ] Manual testing performed

## Checklist
- [ ] Code follows style guidelines
- [ ] Self-review completed
- [ ] Documentation updated
- [ ] Tests pass locally
```

### Review Process

1. All PRs require at least 2 approvals from core contributors
2. Automated CI checks must pass
3. Code review focuses on:
   - Correctness and security
   - Performance implications
   - Code clarity and maintainability
   - Test coverage

## Testing Guidelines

### Unit Tests

- Use the existing test framework (inherited from Bitcoin Core)
- Test both success and failure cases
- Mock external dependencies
- Aim for high code coverage

### Integration Tests

- Test complete workflows
- Verify interaction between components
- Test network behavior and consensus rules

### Performance Tests

- Benchmark critical paths
- Test with large blocks and high transaction volumes
- Verify memory usage and CPU performance

## Security Considerations

### Reporting Security Issues

**DO NOT** report security vulnerabilities through public GitHub issues.

Instead, email security@bitcoindecentral.org with:
- Description of the vulnerability
- Steps to reproduce
- Potential impact assessment
- Suggested fix (if available)

### Security Review Process

- All consensus-related changes require security review
- Cryptographic implementations need expert review
- Network protocol changes require thorough analysis

## Development Environment Setup

### Prerequisites

- GCC 7+ or Clang 5+
- CMake 3.16+
- Boost 1.64+
- Qt 5.9+ (for GUI)
- Python 3.6+ (for tests)

### Build Instructions

```bash
# Clone the repository
git clone https://github.com/bitcoindecentral/btcd.git
cd btcd

# Configure build
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build -j$(nproc)

# Run tests
cd build && ctest
```

## Documentation

### Code Documentation

- Use Doxygen-style comments for public APIs
- Document complex algorithms and data structures
- Include usage examples where appropriate

### User Documentation

- Update relevant documentation for user-facing changes
- Provide migration guides for breaking changes
- Keep README and setup instructions current

## Communication Channels

- **GitHub Issues:** Bug reports and feature requests
- **GitHub Discussions:** General questions and ideas
- **Discord:** Real-time development chat (link TBD)
- **Mailing List:** Technical discussions (link TBD)

## License

By contributing to Bitcoin Decentral, you agree that your contributions will be licensed under the same license as the project (to be determined).

---

## Questions?

If you have questions about contributing, please:
1. Check existing documentation
2. Search closed issues and discussions
3. Ask in the appropriate communication channel
4. Create a GitHub discussion for general questions

Thank you for contributing to Bitcoin Decentral!
