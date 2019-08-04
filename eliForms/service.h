#ifndef serviceH
#define serviceH

enum eWndType
	{
	  eForm = 0,
	  eButton = 1,
	  eEdit = 2,
	  eLabel = 3
    };

class EWndLink
{
  public:
	EWndLink(UINT id, UINT type, TObject *ptr)
	{
	  ewID = id;
      ewType = type;
	  ewPtr = ptr;
	}
	virtual ~EWndLink(){delete ewPtr;}

  private:
	UINT ewID;     //���������� ���������� ELI
	UINT ewType;   //��� ���� (�����, ������ � �.�.)
	TObject *ewPtr;  //��������� �� ������
};

#endif
