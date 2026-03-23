# Compilers Lab - Semester 2026-2 **UNAM**

## Project Overview

This repository contains the incremental development of a compiler implemented in C. The project is structured around 11 focused laboratory practices that serve as fundamental building blocks for the final Semester Integration Project.

I am using C as the sole implementation language to ensure efficient memory management and compatibility with standard development tools. All compiler components are designed to operate within a GNU/Linux environment.

Este repositorio contiene el desarrollo incremental de un compilador usando lenguaje C. El proyecto se estructura con 11 prácticas de laboratorio focalizadas que sirven como los bloques fundamentales para el Proyecto Integrador Semestral final. Estoy utilizando C como único lenguaje de implementación para asegurar una gestión de memoria eficiente y compatibilidad con herramientas de desarrollo estándar. Todos los componentes de este compilador están diseñados para operar en un entorno GNU/Linux.

---

## Branching Strategy

I'll use a Feature Branching strategy to maintain a clear and traceable history of my compiler's evolution. The `main` branch is reserved exclusively for stable code that has passed official technical competence validation.

For each laboratory assignment, I will create a dedicated branch (e.g., `lab-01-lexer`, `lab-05-parser`) where I will perform all implementation, debugging, and testing. Once I complete the requirements for a specific module and receive validation, I will merge that branch into `main` to ensure the repository reflects my progress toward a functional compiler.

Usaré una estrategia de Feature Branching para mantener un historial claro y trazable de la evolución de mi compilador. La rama `main` está reservada exclusivamente para código estable que ha pasado la validación oficial de competencia técnica.

Para cada práctica de laboratorio, crearé una rama dedicada (ej., `lab-01-lexer`, `lab-05-parser`) donde realizaré toda la implementación, depuración y pruebas. Una vez que complete los requisitos de un módulo específico y reciba la validación, integraré dicha rama en `main` para asegurar que el repositorio refleje mi progreso hacia un compilador funcional.

---

## Repository Structure

The project is organized into a `src` directory for compiler module implementations, an `include` directory for header files, and a `labs` directory containing the 11 individual folders for technical reports and specific assignment documentation.

El proyecto está organizado en un directorio `src` para las implementaciones de los módulos del compilador, un directorio `include` para archivos de cabecera, y un directorio `labs` que contiene las 11 carpetas individuales para reportes técnicos y documentación específica de cada práctica.
