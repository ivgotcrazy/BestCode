from django.contrib import admin

from .models import User, PrimaryDepartment, SecondaryDepartment, ProgrammingLanguage

# Register your models here.
admin.site.register(User)
admin.site.register(PrimaryDepartment)
admin.site.register(SecondaryDepartment)
admin.site.register(ProgrammingLanguage)
