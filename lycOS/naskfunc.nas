; �ꍱ�p???�������I����

[FORMAT "WCOFF"]                ; �����?�����͎�
[BITS 32]                       ; ���� 32 �ʊ���?��


; �����?�����M��

[FILE "naskfunc.nas"]           ; ���������M��

        GLOBAL	_io_hlt         ; ��������ܓI������


; �ȉ���??�I����

[SECTION .text]		; �ʗ����?���V�@��?�n�ʒ���

_io_hlt:            ; �� C ���I void io_hlt(void);
        HLT
        RET         ; �ԉ�, �� return;
