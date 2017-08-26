from django.conf.urls import url
from . import views

urlpatterns = [
    url(r'^(?P<submit_id>[0-9]\d*)', views.submit, name='submit'),
    url(r'^praise', views.addPraise, name='addPraise'),
]

