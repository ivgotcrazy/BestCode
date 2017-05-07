from django.contrib import admin

from .models import PrimaryDepartment, SecondaryDepartment, ProgrammingLanguage, ActivityUser
from django.contrib.auth import models as auth_models

class PrimaryDepartmentAdmin(admin.ModelAdmin):
    pass

#admin.site.register(auth_models.User)
admin.site.register(ActivityUser)
admin.site.register(PrimaryDepartment, PrimaryDepartmentAdmin)
admin.site.register(SecondaryDepartment)
admin.site.register(ProgrammingLanguage)
