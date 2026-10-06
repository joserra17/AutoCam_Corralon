#!/usr/bin/env python3
"""Puente USB serie ESP32 -> ordenador -> Arduino Opta."""

from __future__ import annotations

import argparse
import queue
import re
import sys
import threading
import time

import serial
from serial.tools import list_ports

LEVEL_PATTERN = re.compile(r"^LEVEL,\d+,(?:\d|[1-9]\d|100),-?\d+(?:\.\d+)?,-?\d+(?:\.\d+)?$")
ERROR_PATTERN = re.compile(r"^SENSOR_ERROR,\d+$")


def available_ports() -> str:
    ports = [f"{port.device}: {port.description}" for port in list_ports.comports()]
    return "\n".join(ports) if ports else "No se ha detectado ningún puerto serie."


def read_commands(command_queue: queue.Queue[str]) -> None:
    while True:
        try:
            command_queue.put(input())
        except EOFError:
            return


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Reenvía el nivel de una ESP32 al Opta.")
    parser.add_argument("--esp", help="Puerto de la ESP32, por ejemplo COM5")
    parser.add_argument("--opta", help="Puerto del Opta, por ejemplo COM6")
    parser.add_argument("--setpoint", type=int, help="Nivel deseado, de 1 a 100 %%")
    parser.add_argument("--list", action="store_true", help="Muestra los puertos y termina")
    return parser.parse_args()


def validate_setpoint(value: int) -> int:
    if not 1 <= value <= 100:
        raise ValueError("la consigna debe estar entre 1 y 100 %")
    return value


def main() -> int:
    args = parse_args()
    if args.list:
        print(available_ports())
        return 0
    if not args.esp or not args.opta or args.setpoint is None:
        print(available_ports(), file=sys.stderr)
        print("\nFaltan --esp, --opta y/o --setpoint.", file=sys.stderr)
        return 2
    try:
        setpoint = validate_setpoint(args.setpoint)
    except ValueError as error:
        print(f"Error: {error}", file=sys.stderr)
        return 2

    commands: queue.Queue[str] = queue.Queue()
    threading.Thread(target=read_commands, args=(commands,), daemon=True).start()

    try:
        with serial.Serial(args.esp, 115200, timeout=0.1) as esp, serial.Serial(
            args.opta, 115200, timeout=0.1
        ) as opta:
            time.sleep(2.0)
            opta.reset_input_buffer()
            next_setpoint_send = 0.0
            print(
                f"Puente activo: {args.esp} -> {args.opta}; consigna {setpoint} %.\n"
                "Escribe un porcentaje y pulsa Intro para cambiarlo; 'reset' rearma el tiempo máximo."
            )
            while True:
                now = time.monotonic()
                if now >= next_setpoint_send:
                    opta.write(f"SETPOINT,{setpoint}\n".encode("ascii"))
                    next_setpoint_send = now + 1.0

                line = esp.readline().decode("ascii", errors="ignore").strip()
                if LEVEL_PATTERN.fullmatch(line) or ERROR_PATTERN.fullmatch(line):
                    opta.write((line + "\n").encode("ascii"))

                while opta.in_waiting:
                    status = opta.readline().decode("ascii", errors="ignore").strip()
                    if status.startswith(("STATUS,", "OPTA_READY")):
                        print(status)

                try:
                    command = commands.get_nowait().strip().lower()
                except queue.Empty:
                    continue
                if command == "reset":
                    opta.write(b"RESET\n")
                    print("Solicitud de rearme enviada.")
                    continue
                try:
                    setpoint = validate_setpoint(int(command))
                    next_setpoint_send = 0.0
                    print(f"Nueva consigna: {setpoint} %")
                except ValueError:
                    print("Escribe un número de 1 a 100 o 'reset'.")
    except serial.SerialException as error:
        print(f"Error de puerto serie: {error}", file=sys.stderr)
        return 1
    except KeyboardInterrupt:
        print("\nPuente detenido. El Opta apagará la bomba por timeout.")
        return 0


if __name__ == "__main__":
    raise SystemExit(main())
