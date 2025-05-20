# rtl8733bu-20240806
RTL8733BU Linux Driver v5.15.12-123-20240806 FPV Mod  
[rtl8733BU_WiFi_linux_v5.15.12-123-g23d8f6271.20240806_COEX20230616-330e.tar.gz](https://github.com/user-attachments/files/20352072/rtl8733BU_WiFi_linux_v5.15.12-123-g23d8f6271.20240806_COEX20230616-330e.tar.gz)

## What's New
[ReleaseNotes.pdf](https://github.com/user-attachments/files/20352075/ReleaseNotes.pdf)
### Features
 - It seems that the new code contains some 80MHz-BW-related functions. Tried, the MAC seems good, but the RF part is not working. no luck
 
### Bug fixes 
 - MCS2 + LDPC + certain packet length failure: not fixed
 - RX LDPC: not fixed


## Why a new repo & which should I use?
20230626 -> 20240806 contains 300+ file changes, so I decided to do a quick merge of the FPV patches to this version instead of merging to the old repo.  

As there's no new feature (for the wfb-based system), and the bugs have not been fixed, using either of them is ok.  


