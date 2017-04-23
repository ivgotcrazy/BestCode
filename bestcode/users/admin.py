from django.contrib import admin

from .models import UserInfo, PrimaryDepartment, SecondaryDepartment, ProgrammingLanguage
from django.contrib.auth import models as auth_models

class PrimaryDepartmentAdmin(admin.ModelAdmin):
    pass

#admin.site.register(auth_models.User)
admin.site.register(UserInfo)
admin.site.register(PrimaryDepartment, PrimaryDepartmentAdmin)
admin.site.register(SecondaryDepartment)
admin.site.register(ProgrammingLanguage)
