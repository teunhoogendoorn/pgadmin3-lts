//////////////////////////////////////////////////////////////////////////
//
// pgAdmin III - PostgreSQL Tools
// RCS-ID:      $Id$
// Copyright (C) 2002 - 2010, The pgAdmin Development Team
// This software is released under the PostgreSQL Licence
//
// pgLanguage.cpp - Language class
//
//////////////////////////////////////////////////////////////////////////

// wxWindows headers
#include <wx/wx.h>

// App headers
#include "pgAdmin3.h"
#include "utils/misc.h"
#include "schema/pgLanguage.h"


pgLanguage::pgLanguage(const wxString& newName)
: pgDatabaseObject(languageFactory, newName)
{
}

wxString pgLanguage::GetTranslatedMessage(int kindOfMessage) const
{
    wxString message = wxEmptyString;
    
    switch (kindOfMessage)
    {
        case RETRIEVINGDETAILS:
            message = _("Retrieving details on language");
            message += wxT(" ") + GetName();
            break;
        case REFRESHINGDETAILS:
            message = _("Refreshing language");
            message += wxT(" ") + GetName();
            break;
        case DROPINCLUDINGDEPS:
            message = wxString::Format(_("Are you sure you wish to drop language \"%s\" including all objects that depend on it?"),
                GetFullIdentifier().c_str());
            break;
        case DROPEXCLUDINGDEPS:
            message = wxString::Format(_("Are you sure you wish to drop language \"%s?\""),
                GetFullIdentifier().c_str());
            break;
        case DROPCASCADETITLE:
            message = _("Drop language cascaded?");
            break;
        case DROPTITLE:
            message = _("Drop language?");
            break;
        case PROPERTIESREPORT:
            message = _("Language properties report");
            message += wxT(" - ") + GetName();
            break;
        case PROPERTIES:
            message = _("Language properties");
            break;
        case DDLREPORT:
            message = _("Language DDL report");
            message += wxT(" - ") + GetName();
            break;
        case DDL:
            message = _("Language DDL");
            break;
        case DEPENDENCIESREPORT:
            message = _("Language dependencies report");
            message += wxT(" - ") + GetName();
            break;
        case DEPENDENCIES:
            message = _("Language dependencies");
            break;
        case DEPENDENTSREPORT:
            message = _("Language dependents report");
            message += wxT(" - ") + GetName();
            break;
        case DEPENDENTS:
            message = _("Language dependents");
            break;
    }

    return message;
}

bool pgLanguage::DropObject(wxFrame *frame, ctlTree *browser, bool cascaded)
{
    wxString sql=wxT("DROP LANGUAGE ") + GetQuotedFullIdentifier();
    if (cascaded)
        sql += wxT(" CASCADE");
    return GetDatabase()->ExecuteVoid(sql);
}


wxString pgLanguage::GetSql(ctlTree *browser)
{
    if (sql.IsNull())
    {
        sql = wxT("-- Language: ") + GetQuotedFullIdentifier() + wxT("\n\n")
            + wxT("-- DROP LANGUAGE ") + GetQuotedFullIdentifier() + wxT(";")
            + wxT("\n\n CREATE ");
        if (GetTrusted())
            sql += wxT("TRUSTED ");
        sql += wxT("PROCEDURAL LANGUAGE '") + GetName() 
            +  wxT("'\n  HANDLER ") + GetHandlerProc();

        if (!GetValidatorProc().IsEmpty())
            sql += wxT("\n  VALIDATOR ") + GetValidatorProc();

        sql += wxT(";\n")
            +  GetOwnerSql(8, 3, wxT("LANGUAGE ") + GetName())
            +  GetGrant(wxT("U"), wxT("LANGUAGE ") + GetQuotedFullIdentifier());
    }
    return sql;
}


void pgLanguage::ShowTreeDetail(ctlTree *browser, frmMain *form, ctlListView *properties, ctlSQLBox *sqlPane)
{
    if (properties)
    {
        CreateListColumns(properties);

        properties->AppendItem(_("Name"), GetName());
        properties->AppendItem(_("OID"), GetOid());
        if (GetConnection()->BackendMinimumVersion(8, 3))
            properties->AppendItem(_("Owner"), GetOwner());
        properties->AppendItem(_("ACL"), GetAcl());
        properties->AppendItem(_("Trusted?"), GetTrusted());
        properties->AppendItem(_("Handler"), GetHandlerProc());
        properties->AppendItem(_("Validator"), GetValidatorProc());
        properties->AppendItem(_("System language?"), GetSystemObject());
        if (GetConnection()->BackendMinimumVersion(7, 5))
            properties->AppendItem(_("Comment"), firstLineOnly(GetComment()));
    }
}



pgObject *pgLanguage::Refresh(ctlTree *browser, const wxTreeItemId item)
{
    pgObject *language=0;
    pgCollection *coll=browser->GetParentCollection(item);
    if (coll)
        language = languageFactory.CreateObjects(coll, 0, wxT("\n   AND lan.oid=") + GetOidStr());

    return language;
}



pgObject *pgLanguageFactory::CreateObjects(pgCollection *collection, ctlTree *browser, const wxString &restriction)
{
    wxString sql;
    pgLanguage *language=0;

    sql = wxT("SELECT lan.oid, lan.lanname, lanpltrusted, lanacl, hp.proname as lanproc, vp.proname as lanval, description");
    if (collection->GetConnection()->BackendMinimumVersion(8, 3))
        sql += wxT(", pg_get_userbyid(lan.lanowner) as languageowner\n");
    sql += wxT("  FROM pg_language lan\n")
        wxT("  JOIN pg_proc hp on hp.oid=lanplcallfoid\n")
        wxT("  LEFT OUTER JOIN pg_proc vp on vp.oid=lanvalidator\n")
        wxT("  LEFT OUTER JOIN pg_description des ON des.objoid=lan.oid AND des.objsubid=0\n")
        wxT(" WHERE lanispl IS TRUE")
        + restriction + wxT("\n")
        wxT(" ORDER BY lanname");
    pgSet *languages= collection->GetDatabase()->ExecuteSet(sql);

    if (languages)
    {
        while (!languages->Eof())
        {

            language = new pgLanguage(languages->GetVal(wxT("lanname")));
            language->iSetDatabase(collection->GetDatabase());
            language->iSetOid(languages->GetOid(wxT("oid")));
            if (collection->GetConnection()->BackendMinimumVersion(8, 3))
                language->iSetOwner(languages->GetVal(wxT("languageowner")));
            language->iSetAcl(languages->GetVal(wxT("lanacl")));
            language->iSetComment(languages->GetVal(wxT("description")));
            language->iSetHandlerProc(languages->GetVal(wxT("lanproc")));
            language->iSetValidatorProc(languages->GetVal(wxT("lanval")));
            language->iSetTrusted(languages->GetBool(wxT("lanpltrusted")));

            if (browser)
            {
                browser->AppendObject(collection, language);
	
			    languages->MoveNext();
            }
            else
                break;
        }

		delete languages;
    }
    return language;
}


/////////////////////////////

wxString pgLanguageCollection::GetTranslatedMessage(int kindOfMessage) const
{
    wxString message = wxEmptyString;
    
    switch (kindOfMessage)
    {
        case RETRIEVINGDETAILS:
            message = _("Retrieving details on languages");
            break;
        case REFRESHINGDETAILS:
            message = _("Refreshing languages");
            break;
        case OBJECTSLISTREPORT:
            message = _("Languages list report");
            break;
    }
    
    return message;
}

///////////////////////////////////////////////////

#include "images/language.xpm"
#include "images/language-sm.xpm"
#include "images/languages.xpm"

pgLanguageFactory::pgLanguageFactory() 
: pgDatabaseObjFactory(__("Language"), __("New Language..."), __("Create a new Language."), language_xpm, language_sm_xpm)
{
}


pgLanguageFactory languageFactory;
static pgaCollectionFactory cf(&languageFactory, __("Languages"), languages_xpm);
