*** Settings ***
Documentation       Crew-box firmware integration tests: the test image on the
...                 MIMXRT1064-EVK model in Renode, with sim/rig.py as the CCU.
...                 Requirement IDs (CB-xx) refer to fw/SRS.md.
Suite Setup         Setup
Suite Teardown      Teardown
Test Setup          Reset Emulation
Test Teardown       Test Teardown
Resource            common.resource

*** Test Cases ***
IT01 Boot To Normal
    [Documentation]    CB-20, CB-27: INIT -> NORMAL on the first CCU frames; relay energised.
    [Tags]    CB-20    CB-27
    Boot
    Advance    0.05
    State Should Be    ${ST_NORMAL}
    ${t}=    Rig    rig.first_time_in(1)
    Log To Console    \nNORMAL at ${t} ms of virtual time
    ${e}=    Rig    rig.relay_energised()
    Should Be Equal    ${e}    True

IT02 CCU Loss To Bypass
    [Documentation]    CB-22, CB-27: CCU dies -> BYPASS within 20 ms + 1 tick; relay drops.
    [Tags]    CB-22    CB-27
    Boot To Normal
    Rig    rig.ccu(False)
    ${last}=    Rig    rig.last_sent_ms
    Advance    0.1
    ${tb}=    Rig    rig.first_time_in(3, ${last})
    ${drop}=    Rig    rig.relay_drop_ms()
    ${dt}=    Evaluate    float($tb) - float($last)
    ${dr}=    Evaluate    float($drop) - float($last)
    Log To Console    \nlast frame ${last} ms; BYPASS after ${dt} ms; relay dropped after ${dr} ms
    Should Be True    float($tb) > 0 and 19.0 <= $dt <= 22.0
    Should Be True    float($drop) > 0 and $dr <= 24.0
    ${u0}=    Rig    rig.ups
    Advance    0.05
    ${u1}=    Rig    rig.ups
    Should Be True    int($u1) - int($u0) >= 45    # telemetry continues in BYPASS
    State Should Be    ${ST_BYPASS}

IT03 Recovery Needs 2 s Of Good Link
    [Documentation]    CB-23: no chatter on an intermittent link; NORMAL 2 s after the link is good.
    [Tags]    CB-23
    Boot To Normal
    Rig    rig.ccu(False)
    Advance    0.05
    State Should Be    ${ST_BYPASS}
    Rig    rig.thin(2)
    ${t0}=    Rig    rig.ccu(True)
    Advance    3.0
    State Should Be    ${ST_BYPASS}
    ${n}=    Rig    rig.transitions_after(${t0})
    Should Be Equal As Integers    ${n}    0
    Rig    rig.thin(0)
    ${t1}=    Rig    rig.now_ms()
    Advance    2.1
    ${tn}=    Rig    rig.first_time_in(1, ${t1})
    ${dt}=    Evaluate    float($tn) - float($t1)
    Log To Console    \nNORMAL ${dt} ms after the link became good
    Should Be True    float($tn) > 0 and 1999.0 <= $dt <= 2010.0

IT04 Corrupted Frames Are Concealed
    [Documentation]    CB-12, CB-15: 1 frame in 10 corrupted: counted, concealed, no mode change.
    [Tags]    CB-12    CB-15
    Boot To Normal
    ${t0}=    Rig    rig.now_ms()
    Rig    rig.corrupt(10)
    Advance    2.0
    ${n}=    Rig    rig.transitions_after(${t0})
    Should Be Equal As Integers    ${n}    0
    ${crc}=    Rig    rig.tele_item(3)
    ${lost}=    Rig    rig.tele_item(2)
    Log To Console    \nCRC errors ${crc}, frames lost ${lost}
    Should Be True    180 <= int($crc) <= 210
    Should Be True    abs(int($lost) - int($crc)) <= 1    # each corrupted frame counted once
    State Should Be    ${ST_NORMAL}

IT05 ANR Converges On Target
    [Documentation]    CB-40, CB-41: speed-referenced ANR on the RT1064 build: >= 30 dB on the
    ...    plant's three track harmonics within 3 s at 50 km/h; frame processing time.
    [Tags]    CB-40    CB-41    CB-05
    Boot To Normal
    Rig    rig.set_speed(50)
    Advance    3.0
    ${att}=    Rig    rig.tele_item(1)
    ${cyc}=    Rig    rig.read_u32('icom_diag_max_cyc')
    ${us}=    Evaluate    int($cyc) / 600.0
    Log To Console    \nANR attenuation ${att} x0.1 dB; worst icom_box_tick ${cyc} cycles (${us} us at 600 MHz)
    Should Be True    int($att) >= 300
    Should Be True    $us < 1000.0
    State Should Be    ${ST_NORMAL}

IT06 Wrong S_hat Is Caught
    [Documentation]    CB-28, CB-46: maintenance-only S_hat rotation of 150 deg makes the ANR diverge;
    ...    the guard switches it off (DEGRADED), the digital path stays, the output stays bounded.
    [Tags]    CB-28    CB-46    CB-43
    Boot To Normal
    Rig    rig.command(${CMD_MAINT_ENTER})
    Advance    0.01
    State Should Be    ${ST_MAINT}
    Rig    rig.command(${CMD_SHAT_ROT}, 150)
    Advance    0.01
    Rig    rig.command(${CMD_MAINT_EXIT})
    Advance    0.01
    State Should Be    ${ST_NORMAL}
    Rig    rig.set_speed(50)
    ${t0}=    Rig    rig.now_ms()
    Advance    3.0
    State Should Be    ${ST_DEGRADED}
    ${td}=    Rig    rig.first_time_in(2, ${t0})
    ${dt}=    Evaluate    float($td) - float($t0)
    ${peak}=    Rig    rig.read_u32('icom_test_out_peak')
    Log To Console    \nDEGRADED ${dt} ms after the vehicle moved; largest earphone sample ${peak} (clamp 16384)
    Should Be True    0 < int($peak) <= 16384    # anti-noise clamp 0.5 FS (CB-43)
    ${e}=    Rig    rig.relay_energised()
    Should Be Equal    ${e}    True

IT07 Critical BIT Latches Bypass
    [Documentation]    CB-24: codec BIT fault -> BYPASS, latched although the link is good.
    [Tags]    CB-24
    Boot To Normal
    Rig    rig.write_u32('icom_test_bits', 1)
    Advance    0.01
    State Should Be    ${ST_BYPASS}
    Rig    rig.write_u32('icom_test_bits', 0)
    Advance    3.0
    State Should Be    ${ST_BYPASS}

IT08 Processor Stall Drops The Relay
    [Documentation]    CB-27: the audio thread hangs; the relay drive stops toggling and the relay
    ...    drops to the emergency line within 1 tick + the charge-pump hold time.
    [Tags]    CB-27
    Boot To Normal
    ${t0}=    Rig    rig.now_ms()
    Rig    rig.write_u32('icom_test_stall', 1)
    Advance    0.02
    ${drop}=    Rig    rig.relay_drop_ms()
    ${dt}=    Evaluate    float($drop) - float($t0)
    Log To Console    \nrelay dropped ${dt} ms after the stall
    Should Be True    float($drop) > 0 and $dt <= 3.5
