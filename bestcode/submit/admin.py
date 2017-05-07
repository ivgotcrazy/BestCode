from django.contrib import admin

from .models import Submit, SubmitFile, SubmitFileType, SubmitReviewPlan

# Register your models here.
admin.site.register(Submit)
admin.site.register(SubmitFile)
admin.site.register(SubmitFileType)
admin.site.register(SubmitReviewPlan)
