#include <wx/bitmap.h>
#include <wx/version.h>

#if !(wxMAJOR_VERSION==2 && wxMINOR_VERSION==9 && wxRELEASE_NUMBER==3)
#  error "you need wxWidgets-2.9.3 installed"
#endif

int main()
{
	wxBitmap bitmap;
	return bitmap.GetSize().GetWidth();
}
