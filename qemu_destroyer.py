import termios
import pexpect

def _set_termios_raw():
    """set current tty not to process newlines"""
    attr = termios.tcgetattr(1)

    # disable any newline manilpulations in c_oflag
    attr[1] &= ~(termios.ONLCR | termios.OCRNL | termios.ONOCR | termios.ONLRET)

    termios.tcsetattr(1, termios.TCSANOW, attr)


while True:
    with open("logs", "w") as f:
        dut = pexpect.spawn('./scripts/armv7a9-zynq7000-qemu.sh', preexec_fn=_set_termios_raw, encoding="utf-8", logfile=f)
        dut.expect_exact("root@?:~ # ", timeout=180)
        dut.close()
        print("FOUND")
