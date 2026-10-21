#pragma once
#pragma once
#ifdef __cplusplus
extern "C" {
#endif
	unsigned char* PbrbDecodeImage(const unsigned char* bytes, int length, int* w, int* h);
	void PbrbFreeDecoded(void* p);
#ifdef __cplusplus
}
#endif
