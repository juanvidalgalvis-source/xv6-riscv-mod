
---

# xv6-riscv (fork académico)

> Documentación original de xv6 disponible en [README-xv6.md](./README-xv6.md).

Este repositorio es un derivado de `xv6-riscv` del MIT PDOS (`https://github.com/mit-pdos/xv6-riscv`), usado como base académica para un proyecto del curso de Sistemas Operativos. El punto de partida fue el commit `1982fd12595f52a0e5ef8db466257a01fb1fbfef` de la rama `riscv` del repositorio oficial (`origin/riscv`), correspondiente al estado del proyecto al momento de la clonación.

Autor: JUAN FELIPE VIDAL GALVIS, código 2439854  
Presentado al profesor JEFFERSON AMADO PENA, curso de Sistemas Operativos

## Descripción

Contiene dos modificaciones principales sobre el xv6 original:

1. Scheduler por prioridad estática con envejecimiento (`aging`)
   - Cada `struct proc` tiene `priority` y `age`.
   - El scheduler recorre la tabla de procesos, incrementa `age` de los procesos `RUNNABLE`, calcula una prioridad efectiva y elige el proceso con menor `effective_priority`.
   - El proceso elegido resetea `age = 0` al ejecutarse.

2. Cuota de memoria física por proceso en `sbrk()` eager
   - Cada `struct proc` tiene `mem_pages` y `mem_limit`.
   - `growproc()` verifica antes de asignar memoria que `mem_pages + pages_needed <= mem_limit`.
   - Si la asignación es exitosa, `mem_pages` se actualiza.
   - La cuota solo aplica al crecimiento de heap eager via `sbrk()`.

Se agregaron los siguientes syscalls:
- `set_priority(pid, prio)` → `SYS_set_priority = 23`
- `set_memlimit(pid, pages)` → `SYS_set_memlimit = 24`

## Estructura de ramas

- `main`
  - Línea base / control con el benchmark `bench.c`.
  - Kernel de xv6 original, sin ninguna modificación, más el programa bench.c como carga de prueba.
  - Sirve como línea base para comparar el comportamiento del sistema sin las modificaciones de scheduler y memoria.
- `modificado`
  - Contiene el kernel modificado y benchmarks para ejercicio real.
  - Incluye `bench.c` y `bench_mod.c`.

Esto permite comparar el comportamiento antes/después usando la misma base de proyecto y el mismo flujo de compilación.

## Dependencias

El `Makefile` requiere:
- Toolchain RISC-V: `riscv64-unknown-elf-` / `riscv64-linux-gnu-` / equivalente
- `qemu-system-riscv64`
- `make`
- `gcc`/`ld` para `mkfs`

El desarrollo y la verificación se hicieron en WSL2 sobre Windows, pero el repositorio debe ser compatible con Linux si tiene las dependencias instaladas.

## Compilar y ejecutar

```sh
git clone <repo-url>
cd xv6-riscv
```

Para comparar ramas, cambie de rama y recompila limpio:

```sh
git checkout main
make clean
make
make qemu
```

```sh
git checkout modificado
make clean
make
make qemu
```

`make clean` antes de `make qemu` es importante al cambiar de rama, porque `fs.img` guarda los binarios de la rama anterior.

Para salir de QEMU:
- `Ctrl+A` seguido de `X`

## Cómo probar las modificaciones

- `bench`
  - Ejecuta en ambas ramas como carga de referencia. Crea `CHILDREN = 6` procesos hijos (el mismo número que `bench_mod`, para que la comparación sea directa)
  - Muestra progreso por hijo y páginas asignadas antes de la falla.

- `bench_mod`
  - Diseñado para la rama `modificado`.
  - El padre crea seis hijos (`CHILDREN = 6`), asigna prioridades y cuotas de memoria de forma cíclica:
    - Alta: prioridad `2`, cuota `200`
    - Media: prioridad `10`, cuota `50`
    - Baja: prioridad `18`, cuota `10`
  - Cada hijo espera a que el padre le asigne su configuración antes de comenzar.

Qué observar:
- Mensajes de progreso de cada hijo.
- Cuántas páginas logra asignar cada hijo antes de que `sbrk()` falle.
- Ticks totales transcurridos (`uptime()` antes y después).

## Alcance

La cuota de memoria está limitada a:
- `sbrk()` eager / `growproc()`
- `uvmalloc()` / `uvmdealloc()`

No cubre:
- `fork()` / `uvmcopy()`
- `sbrk()` en modo lazy
- páginas de tabla de páginas intermedias

Esto es una decisión de diseño documentada en el código.

## Licencia

Mantiene la licencia MIT original de xv6:
- El archivo `LICENSE` del repositorio es la MIT License usada por xv6.
- El trabajo es derivado del proyecto xv6 original del MIT PDOS.

---

