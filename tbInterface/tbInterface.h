// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� TBINTERFACE_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// TBINTERFACE_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�
#ifdef TBINTERFACE_EXPORTS
#define TBINTERFACE_API __declspec(dllexport)
#else
#define TBINTERFACE_API __declspec(dllimport)
#endif

// �����Ǵ� tbInterface.dll ������
class TBINTERFACE_API CtbInterface {
public:
	CtbInterface(void);
	// TODO: �ڴ�������ķ�����
};

extern TBINTERFACE_API int ntbInterface;

TBINTERFACE_API int fntbInterface(void);
