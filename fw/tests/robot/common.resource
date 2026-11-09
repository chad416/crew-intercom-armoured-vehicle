*** Settings ***
Documentation       Shared keywords for the crew-box integration tests in Renode.
Resource            ${RENODEKEYWORDS}

*** Variables ***
# Overridden by scripts/test-sim.sh
${ELF}              /root/recorder-ws/build/icom/zephyr/zephyr.elf
${OUT}              /tmp/icom-robot
${SIM}              ${CURDIR}/../../sim
${ST_NORMAL}        1
${ST_DEGRADED}      2
${ST_BYPASS}        3
${ST_MAINT}         4
${CMD_MAINT_ENTER}  1
${CMD_MAINT_EXIT}   2
${CMD_SHAT_ROT}     3

*** Keywords ***
Boot
    Execute Command             $elf=@${ELF}
    Execute Command             include @${SIM}/icom.resc
    Execute Command             include @${SIM}/rig.py
    Execute Command             python "rig = Rig(self.Machine)"
    Create Terminal Tester      sysbus.lpuart1    defaultPauseEmulation=True
    Start Emulation
    Wait For Line On Uart       icom READY    timeout=5

Advance
    [Documentation]    Run the emulation for a fixed virtual time (seconds).
    [Arguments]    ${seconds}
    Execute Command             pause
    Execute Command             emulation RunFor "${seconds}"

Rig
    [Documentation]    Evaluate an expression on the rig (IronPython inside Renode).
    [Arguments]    ${code}
    ${out}=    Execute Command    python "print(${code})"
    ${out}=    Evaluate    $out.strip()
    RETURN    ${out}

State Should Be
    [Arguments]    ${state}
    ${s}=    Rig    rig.state
    Should Be Equal As Integers    ${s}    ${state}

Boot To Normal
    Boot
    Advance    0.3
    State Should Be    ${ST_NORMAL}
