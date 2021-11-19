#include <wtypes.h>

namespace procademy
{
	struct DB_chunk
	{
		USHORT	type;
		char	memory[1022];
	};

#define dfCreateUser (1)
	struct Create_User
	{
		USHORT type = dfCreateUser;
		UINT32 accountID;
		WCHAR name[20];
		UINT32 age;
	};

#define dfSelectUser (2)
	struct Select_User
	{
		USHORT type = dfSelectUser;
		UINT32 accountID;
	};

#define dfCreateItem (3)
	struct Create_Item
	{
		USHORT type = dfCreateItem;
		UINT32 accountID;
		UINT32 itemType;
		UINT32 count;
	};

#define dfSelectItem (4)
	struct Create_Item
	{
		USHORT type = dfSelectItem;
		UINT32 accountID;
		UINT32 itemType;
	};
}