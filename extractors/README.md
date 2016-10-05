# Qualcomm Binaries
Binary makefile generation scripts

## Graphics:
### Branch compiled from:
msm8916: LA.BR.1.2.6_rb1.7  
msm8974: LA.BF.1.1.3_rb1.5  
msm8992: assumed<sup>[1](#googleEGL)</sup> LA.BF64.1.2.3_rb1.2  
msm8994: assumed<sup>[1](#googleEGL)</sup> LA.BF64.1.2.3_rb1.2  
msm8996: LA.UM.5.5_rb1.10
### Origins:
msm8916: 6.0.1 blobs from kipper/crackling  
msm8974: 6.0.1 blobs from bacon  
msm8992: 7.0 blobs from bullhead nrd90s  
msm8994: 7.0 blobs from angler nrd90u  
msm8996: 7.0 blobs from gemini 6.10.13 nrd90m  
### Min kernel patch level required:
msm8916: Any 5.0+ kernel  
msm8974: Any 5.1+ kernel  
msm8992: Any 6.0+ kernel  
msm8994: Any 6.0+ kernel  
msm8996: Patched mdss/kgsl up to LA.UM.5.5.r1-00100-8x96.0  

### Notes:
<a name="googleEGL">1</a>: Google likes to strip QUIC branches from their shipped binaries so we can't really know which branch they originate from
