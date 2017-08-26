from django.conf.urls import url
from . import views

urlpatterns = [
    url(r'^agree$', views.agree, name='agree'),
    url(r'^$', views.comment, name='comment'),
]

